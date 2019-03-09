const baseFetch = require('cross-fetch');

module.exports = function fetch(...args) {
  return baseFetch(...args)
    .then(({
      status
    }) => {
      if (status >= 400) {
        throw new Error("Bad response from server");
      }
    });
}