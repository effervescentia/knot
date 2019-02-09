var React = require('react');
var ReactDOM = require('react-dom');

module.exports = {
  main: {
    createElement: React.createElement,
    render: function (app, id) {
      ReactDOM.render(app, document.getElementById(id))
    }
  }
};