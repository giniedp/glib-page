'use strict';

var gulp = require('gulp');
var del = require('del');
var path = require('path');
var jade = require('gulp-pug');
var sass = require('gulp-sass');
var concat = require('gulp-concat');
var plumber = require('gulp-plumber');
var webserver = require('gulp-webserver');
var livereload = require('gulp-livereload');
var glibPages = require('./tools/glib-samples.js');
var shelljs = require('shelljs');

var PATHS = {
  dist: 'dist',
  assets: [
    'src/assets/**/*'
  ],
  page: {
    pages: [
      '!src/_layouts/*.pug',
      '!src/_includes/*.pug',
      '!src/_temp/*.pug',
      '!src/**/_*.pug',
      'src/*/introduction/*.pug',
      'src/*/graphics-*/*.pug',
      'src/*/input/**/*.pug',
      'src/*/content/**/*.pug',
      'src/*/**/*.pug',
      'src/**/*.pug'
    ],
    fonts: [
      'node_modules/mdi/fonts/*',
    ],
    scripts: [
      'node_modules/jquery/dist/jquery.js',
      'node_modules/prismjs/prism.js',
      'node_modules/prismjs/components/prism-glsl.js',
      'node_modules/prismjs/components/prism-css.js',
      'node_modules/prismjs/components/prism-markup.js',
      'node_modules/prismjs/components/prism-javascript.js',
      'node_modules/prismjs/plugins/autolinker/prism-autolinker.js',
      'node_modules/prismjs/plugins/line-numbers/prism-line-numbers.js',
      'node_modules/prismjs/plugins/normalize-whitespace/prism-normalize-whitespace.js',
      'src/**/*.js'
    ],
    styles: [
      'src/page.scss'
    ],
    stylesWatch: [
      'src/**/*.scss'
    ],
  }
};


function src(){
  return gulp.src.apply(gulp, arguments)
  .pipe(plumber(function (err) {

    console.error(err.message || err);
    if (err.backtrace) {
      console.log(err.backtrace);
    }
  }));
}

function dest(){
  var target = PATHS.dist;
  if (arguments.length) {
    var join = path.join;
    target = join(target, path.join.apply(path, arguments));
  }
  return gulp.dest(target);
}

gulp.task('clean', function(){
  return del([PATHS.dist]);
});

gulp.task('fonts', function(){
  return src(PATHS.page.fonts)
    .pipe(dest('fonts'));
});

gulp.task('scss', function(){
  return src(PATHS.page.styles)
    .pipe(sass({
      includePaths: 'node_modules'
    }).on('error', sass.logError))
    .pipe(concat('page.css'))
    .pipe(dest())
    .pipe(livereload());
});

gulp.task('jade', function(){
  return src(PATHS.page.pages)
    .pipe(glibPages.sampler('src/samples'))
    .pipe(jade({
      jade: require('pug'),
      pretty: true,
      locals: {
        samples: glibPages.samples
      }
    }))
    .pipe(dest())
    .pipe(concat('views.html'))
    .pipe(livereload());
});

gulp.task('scripts', function(){
  shelljs.exec(`
    rm ${__dirname}/dist/glib.js
    ln -s ${__dirname}/../glib/dist/glib.js ${__dirname}/dist/glib.js
    rm ${__dirname}/dist/assets
    ln -s ${__dirname}/assets ${__dirname}/dist/assets
  `);
  return src(PATHS.page.scripts)
    .pipe(concat('page.js'))
    .pipe(dest())
    .pipe(livereload());
});

gulp.task('page', ['fonts', 'scripts', 'scss', 'jade']);

gulp.task('watch', ['page'], function() {
  gulp.watch(PATHS.page.pages, ['page']);
  gulp.watch(PATHS.page.stylesWatch, ['scss']);
});

gulp.task('serve', ['watch'], function() {
  gulp.src(['dist'])
    .pipe(webserver({
      host: '0.0.0.0',
      port: 3000,
      livereload: true,
      directoryListing: false,
      open: false
    }));
});
