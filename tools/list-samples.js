(function(){
  'use strict';

  var fs = require('fs');
  var path = require('path');
  var util = require('gulp-util');
  var through = require('through');
  var inflect = require('inflection');
  var _ = require('underscore');

  function extractMeta(content) {
    var result = {};
    content = content.trim().split('\n');
    var regex = /\/\/-\s*(\w+)\s*:\s*(.+)/;
    for (var i = 0; i < content.length; i++) {
      var line = content[i].trim();
      if (line.substr(0, 2) !== '//') {
        break;
      }
      var match = line.match(regex);
      if (!match) {
        continue;
      }
      result[match[1]] = match[2];
    }
    return result;
  }

  function makeTitle(token) {
    return inflect.humanize(token.replace('-', '_'));
  }
  function makeFileTitle(file) {
    var extName = path.extname(file);
    var baseName = path.basename(file, extName);
    return makeTitle(baseName);
  }

  function makeFileHref(file, dir) {
    file = path.relative(dir, file);
    var extName = path.extname(file);
    var baseName = path.basename(file, extName);
    var dirName = path.dirname(file);
    return path.join(dirName, baseName + '.html');
  }

  function sortSamples(node) {
    node.files.sort((a, b) => (Number(a.weight)||1000) - (Number(b.weight)||1000) );
    node.folders.sort((a, b) => a.title < b.title ? -1 : 1);
    node.folders.forEach(sortSamples);
  }

  function processFiles(files, dir, result) {
    if (!result) {
        throw 'invalid argument "result"';
    }

    files.forEach(function(file) {
      if (!file.startsWith(dir)) {
          return;
      }

      var content = fs.readFileSync(file, 'UTF-8').toString();
      var meta = extractMeta(content);
      if (meta.ignore) {
          return;
      }

      meta.name = path.basename(file);
      meta.title = meta.title || makeFileTitle(file);
      meta.href = meta.href || makeFileHref(file, dir);

      var tokens =  file.substring(dir.length, file.length).split('/');
      tokens.pop(); // pop file name
      var node = result;

      while (tokens.length) {
        var name = tokens.shift();
        if (!name) {
          continue;
        }
        var found = _.findWhere(node.folders, { name: name });
        if (!found) {
          found = {
            name: name,
            title: makeTitle(name),
            files: [],
            folders: []
          };
          node.folders.push(found);
        }
        node = found;
      }

      node.files = node.files || [];
      node.files.push(meta);
    });
    return result;
  }

  function Mod() {
    this.samples = {};
  }

  Mod.prototype.clear = function() {
    Object.keys(this.samples).forEach(function(key) {
      delete this.samples[key];
    }, this);
    _.extend(this.samples, {
      files: [],
      folders: []
    });
  };

  Mod.prototype.sampler = function(samplesDir) {
    var $this = this;

    this.clear();
    if (!samplesDir) {
      throw new util.PluginError('glib-samples', 'Missing samplesDir parameter');
    }
    samplesDir = path.join(process.cwd(), samplesDir);
    var sampleFiles = [];
    var files = [];

    function bufferContents(file) {
      files.push(file);
      if (file.isNull()) {
        return;
      }
      if (file.path.startsWith(samplesDir)) {
        sampleFiles.push(file.path);
      }
    }

    function endStream() {
      processFiles(sampleFiles, samplesDir, $this.samples);
      sortSamples($this.samples);
      files.forEach((file) => this.emit('data', file), this);
      this.emit('end');
    }

    return through(bufferContents, endStream);
  };

  module.exports = new Mod();
}());
