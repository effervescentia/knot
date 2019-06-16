var utils = require('./utils');

var PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

function prop(props, name, defaultVal) {
  return utils.destructureWithDefault(props, name, defaultVal);
}

function arg(argumentsObj, index, name, defaultVal) {
  if (argumentsObj.length === 0) return;

  var hasDefaults = argumentsObj.length > 3;
  var hasParamMap =
    argumentsObj.length === 1 &&
    typeof argumentsObj[0] === 'object' &&
    PARAMETER_MAP in argumentsObj[0];
  var [key, args] = hasParamMap
    ? [name, argumentsObj[0]]
    : [index, argumentsObj];

  if (hasDefaults) {
    return utils.destructureWithDefault(args, key, defaultVal);
  } else {
    return args[key];
  }
}

function style(stylePlugin, styles) {
  const jss = {};

  for (var key in styles) {
    if (!styles.hasOwnProperty(key) || key[0] !== '.') break;

    jss[key.slice(1)] = styles[key];
  }

  return stylePlugin.resolve(jss);
}

module.exports = {
  prop: prop,
  arg: arg,
  style: style
};
