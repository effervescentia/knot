const PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

module.exports = {
  arg: function (argumentsObj, index, name, defaultVal) {
    if (argumentsObj.length === 0) return;

    const hasDefaults = argumentsObj.length > 3;
    const hasParamMap = argumentsObj.length === 1 && typeof argumentsObj[0] === 'object' && PARAMETER_MAP in argumentsObj[0];
    const [key, args] = hasParamMap ? [name, argumentsObj[0]] : [index, argumentsObj];

    if (hasDefaults) {
      return key in args ? args[key] : defaultVal;
    } else {
      return args[key];
    }
  }
};