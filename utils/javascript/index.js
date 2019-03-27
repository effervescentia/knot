const PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

function destructureWithDefault(obj, key, defaultVal) {
  return key in obj ? obj[key] : defaultVal;
}

module.exports = {
  prop: function (props, name, defaultVal) {
    return destructureWithDefault(props, name, defaultVal);
  },
  arg: function (argumentsObj, index, name, defaultVal) {
    if (argumentsObj.length === 0) return;

    const hasDefaults = argumentsObj.length > 3;
    const hasParamMap = argumentsObj.length === 1 && typeof argumentsObj[
      0] === 'object' && PARAMETER_MAP in argumentsObj[0];
    const [key, args] = hasParamMap ? [name, argumentsObj[0]] : [index,
      argumentsObj
    ];

    if (hasDefaults) {
      return destructureWithDefault(args, key, defaultVal);
    } else {
      return args[key];
    }
  }
};
