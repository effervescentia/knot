import * as React from 'react';
import * as ReactDOM from 'react-dom';

export const main = {
  createElement: React.createElement,
  render(app: JSX.Element, id: string): void {
    // tslint:disable-next-line:no-expression-statement
    ReactDOM.render(app, document.getElementById(id));
  }
};
