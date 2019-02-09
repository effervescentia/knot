var moize = require('moize').default;

var PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

module.exports = function javascriptUtils(stylePlugin) {
  return {
    memo: moize,

    arg: moize(function (argumentsObj, index, name, defaultVal) {
      if (argumentsObj.length === 0) return;

      var hasDefaults = argumentsObj.length > 3;
      var hasParamMap = argumentsObj.length === 1 && typeof argumentsObj[0] === 'object' && PARAMETER_MAP in argumentsObj[0];

      var found = hasParamMap ? [name, argumentsObj[0]] : [index, argumentsObj];
      var key = found[0];
      var args = found[1];

      if (hasDefaults) {
        return key in args ? args[key] : defaultVal;
      } else {
        return args[key];
      }
    }),

    style: moize(function (styles) {
      var jss = {};

      for (const key in styles) {
        if (!styles.hasOwnProperty(key) || key[0] !== '.') break;

        jss[key.slice(1)] = styles[key];
      }

      return stylePlugin.resolve(jss);
    })
  };
};