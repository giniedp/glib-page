'use strict';

//
// UTILS
//

// escapes HTML entities
function escapeHtml(html){
  return String(html)
    .replace(/&/g, '&amp;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function fixIndents(string) {
  var lines = string.split('\n');
  // skip if string is empty
  if (!lines.length) {
    return string;
  }
  // fixes 'outerHTML' indents
  // the first line of outerHTML wont have any indent but last line will.
  // Copy indent from last line to first line
  if (lines[0].match(/^<script/) && lines[lines.length-1].match(/\s+<\/script>$/)) {
    lines[0] = lines[lines.length-1].match(/(\s+)<\/script>$/)[1] + lines[0];
  }
  // find smallest indent value
  var minIndent = Number.MAX_VALUE;
  lines.forEach(function(line) {
    var match = line.match(/^(\s*)/);
    // if line is not blank
    if (match[1].length !== line.length) {
      minIndent = Math.min(minIndent, match[1].length);
    }
  });

  // remove the smalles indent from all lines
  // and join to string
  return lines.map(function(line) {
    return line.substring(minIndent);
  }).join('\n');
}

function explainCode(js) {

  // replace tabs with spaces
  js = js.replace(/[\t]/g, '  ');
  // normalize line feeds
  js = js.replace(/[\r\n]/g, '\n');

  // Now break lines up into sections
  var sections = [];
  var lines = js.split(/\n/m);
  var section = null;
  var wasComment = true;

  function nextSection() {
    // add new section if previous is null or empty
    if (!section || section.comments.length || section.code.length) {
      section = { comments:[], code:[] };
      sections.push(section);
    }
  }

  nextSection();
  lines.forEach(function (line) {

    var isBlank = /^\s*$/.test(line);
    if (isBlank) {
      if (wasComment) {
        nextSection();
      } else {
        section.code.push(line); // retain lines between code blocks
      }
      return;
    }

    // add next comment or code
    var commentMatch = line.match(/^\s*\/\/(.*)/);
    if (commentMatch && !commentMatch[1].match(/^\s*@/) && !commentMatch[1].match(/^\//)) {
      if (!wasComment) {
        nextSection();
      }
      section.comments.push(commentMatch[1]);
      wasComment = true;
    } else {
      section.code.push(line);
      wasComment = false;
    }
  });

  return sections.map(function(section) {
    return {
      comments: fixIndents(section.comments.join('\n')),
      code: section.code.join('\n')
    };
  });
}

if (Prism.plugins.NormalizeWhitespace) {
  Prism.plugins.NormalizeWhitespace.setDefaults({
    'remove-trailing': true,
    'remove-indent': false,
    'left-trim': false,
    'right-trim': true,
  });
}

function makeSnippet(code, language) {
  var snippet = $('<pre>')
      .addClass('language-' + language)
      .addClass('line-numbers')
      .append('<code>');
  var codeEl = snippet.find('code');
  Prism.highlightElement(codeEl.text(code)[0]);
  return snippet;
}

function makeAnnotatedSnippet(code, language) {
  var converter = new showdown.Converter();
  var root = $('<ul>').addClass('annotated-code');
  explainCode(code).map(function(section) {
    var node = $('<li>').addClass('annotated-section').appendTo(root);
    var annotation = $('<div>').addClass('annotation').appendTo(node);
    annotation.html(converter.makeHtml(escapeHtml(section.comments)));
    makeSnippet(section.code, language).appendTo(node);
  });
  return root;
}

function getItem(key) {
  if (window.localStorage) {
    return window.localStorage.getItem(key);
  }
}
function setItem(key, value) {
  if (window.localStorage) {
    window.localStorage.setItem(key, value);
  }
}


var GlibSamples = GlibSamples || {};
(function(module) {

  module.bootSample = function(options) {
    var view = $(options.view);
    var code = $(options.code);
    var source = $(options.source);

    var tabs = [];
    tabs.push({
      name: 'RESULT',
      index: 0,
      content: view
    });

    function currentTab() {
      var tab = tabs[tabs.length-1] || {
        name: 'Example',
        content: $('<div>')
      };
      tabs[tabs.length-1] = tab;
      return tab;
    }
    source.children().each(function() {
      var $this = $(this);
      if ($this.is('a[name]')) {
        tabs.push({
          name: $this.attr('name'),
          index: Number($this.attr('index'))|0,
          content: $('<div>')
        });
      } else if ($this.is('viewcode')) {
        makeSnippet(fixIndents(view[0].innerHTML), 'html').appendTo(currentTab().content);
      } else if ($this.is('script') && !$this.is('[no-example]')) {
        if ($this.is('[type="text/javascript"]')) {
          makeAnnotatedSnippet(fixIndents($this[0].innerHTML), 'js').appendTo(currentTab().content);
        } else {
          makeSnippet(fixIndents($this[0].outerHTML), 'html').appendTo(currentTab().content);
        }
      } else if ($this.is('style')) {
        makeSnippet(fixIndents($this[0].innerHTML), 'css').appendTo(currentTab().content);
      } else if ($this.is('[example]')) {
        $this.detach().appendTo(currentTab().content);
        makeAnnotatedSnippet(fixIndents($this[0].innerHTML), 'js').appendTo(currentTab().content);
      } else {
        $this.detach().appendTo(currentTab().content);
      }
    });

    var tabsEl = $('<div>').addClass('sample-tabs').appendTo(code);
    var contentsEl = $('<div>').addClass('sample-contents').appendTo(code);

    tabs.sort(function(a, b) {
      return a.index - b.index;
    }).forEach(function(tab) {
      var tabEl = $('<a>').text(tab.name).attr('tab', tab.name).appendTo(tabsEl);
      var contentEl = tab.content.addClass('tab-content').appendTo(contentsEl);
      tabEl.click(function() {
        setItem('recentTab', tab.name);
        contentsEl.children().hide();
        contentEl.show();
        tabsEl.children().removeClass('active');
        tabEl.addClass('active');
      });
    });

    var recent = getItem('recentTab');
    var found = tabsEl.find('[tab="' + recent + '"]');
    if (recent && found.length) {
      found.click();
    } else {
      tabsEl.find('[tab]').first().click();
    }
  };

  //
  // Sample Menu
  //

  module.sampleLinks = [];
  module.sampleIframe = null;
  module.sampleTitle = null;
  module.onHashChanged = function() {
    var hash = location.hash;
    if (!hash) {
      return;
    }
    var link = module
      .sampleLinks
      .removeClass('active')
      .filter('[href="' + hash + '"]')
      .addClass('active');
    module
      .sampleIframe
      .fadeTo(200, 0, function() {
        $(this)
          .attr('src', hash.replace('#', '/samples/'))
          .delay(300)
          .fadeTo(200, 1);
      });
    module.sampleTitle
      .fadeTo(200, 0, function() {
        $(this)
          .text(link.attr('title') || link.text())
          .delay(300)
          .fadeTo(200, 1);
      });
  };
  module.openFirstSample = function() {
    location.hash = module.sampleLinks.first().attr('href');
  };
  module.openNextSample = function() {
    var links = module.sampleLinks;
    var link = links.filter('.active').first();
    if (!link.length) {
      module.openFirstSample();
      return;
    }

    link = $(links[links.index(link) + 1]);
    if (!link.length) {
      module.openFirstSample();
      return;
    }
    location.hash = link.attr('href');
  };
  module.openPrevSample = function() {
    var links = module.sampleLinks;
    var link = links.filter('.active').first();
    if (!link.length) {
      module.openFirstSample();
      return;
    }

    link = $(links[links.index(link) - 1]);
    if (!link.length) {
      module.openFirstSample();
      return;
    }
    location.hash = link.attr('href');
  };
  module.bootSampleMenu = function(options) {
    module.sampleLinks = $(options.links);
    module.sampleIframe = $(options.iframe);
    module.sampleTitle = $(options.title);
    window.addEventListener('hashchange', module.onHashChanged, false);
    if (location.hash) {
      module.onHashChanged();
    } else {
      module.openFirstSample();
    }
  };
}(GlibSamples));
