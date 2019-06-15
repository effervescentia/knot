export default {
  failFast: true,
  files: ['build/main/**/*.test.js'],
  sources: ['build/main/**/*.js'],
  require: ['./ava-setup.js']
};
