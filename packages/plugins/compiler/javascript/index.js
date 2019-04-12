var PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

function destructureWithDefault(obj, key, defaultVal) {
  return key in obj ? obj[key] : defaultVal;
}

module.exports = {
  prop: function (props, name, defaultVal) {
    return destructureWithDefault(props, name, defaultVal);
  },
  arg: function (argumentsObj, index, name, defaultVal) {
    if (argumentsObj.length === 0) return;

    var hasDefaults = argumentsObj.length > 3;
    var hasParamMap = argumentsObj.length === 1 && typeof argumentsObj[
      0] === 'object' && PARAMETER_MAP in argumentsObj[0];
    var [key, args] = hasParamMap ? [name, argumentsObj[0]] : [index,
      argumentsObj
    ];

    if (hasDefaults) {
      return destructureWithDefault(args, key, defaultVal);
    } else {
      return args[key];
    }
  },
  style: function (stylePlugin, styles) {
    const jss = {};

    for (var key in styles) {
      if (!styles.hasOwnProperty(key) || key[0] !== '.') break;

      jss[key.slice(1)] = styles[key];
    }

    return stylePlugin.resolve(jss);
  }
};
