#!/usr/bin/env node

const install = (function() {
  try {
    return require('./build/main/install').default;
  } catch (e) {
    console.log('install is not possible until project has been built');
  }
})();

install && install(__dirname);
