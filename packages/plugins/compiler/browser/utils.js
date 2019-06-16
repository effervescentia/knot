function destructureWithDefault(obj, key, defaultVal) {
  return key in obj ? obj[key] : defaultVal;
}

module.exports = {
  destructureWithDefault: destructureWithDefault
};
