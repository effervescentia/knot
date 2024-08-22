import * as React from 'react';
import * as ReactDOM from 'react-dom/client';

export const createElement = {
  "1.0": (...args) => React.createElement(...args),
};

export const mount = {
  "1.0": (element, id) => {
    const root = ReactDOM.createRoot(document.getElementById(id));
    root.render(element);

    return () => root.unmount();
  },
}
