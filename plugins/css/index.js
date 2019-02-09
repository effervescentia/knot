var JSS = require('jss').default;
var preset = require('jss-preset-default').default;

JSS.setup(preset());

module.exports = {
  main: {
    fontSize: 'fontSize',
    color: 'color',
    backgroundColor: 'backgroundColor',
    visibility: 'visibility',
    display: 'display',

    hidden: 'hidden',

    flex: 'flex',

    red: 'red',

    px: function (num) {
      return String(num) + 'px'
    }
  },

  resolve: function (styles) {
    var attached = JSS.createStyleSheet(styles).attach();

    return attached.classes;
  }
};