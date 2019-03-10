import * as React from 'react';
import * as ReactDOM from 'react-dom';

export const main = {
  createElement: React.createElement,
  render(app: JSX.Element, id: string) {
    ReactDOM.render(app, document.getElementById(id));
  },
};
