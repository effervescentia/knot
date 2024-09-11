import { $knot } from "@knot/runtime";
import * as React from 'react';
import * as ReactDOM from 'react-dom/client';

const combineClassNames = (root, extension) => {
  return root && extension ? root + " " + extension : root || extension;
};

const transformProps = ({ class: class_, style, ...props }) => {
  const styleClass = style && $knot.plugin.get("style", "bindStyle", "1.0")(style);

  return {
    ...props,
    className: combineClassNames(styleClass, class_),
  };
};

export const createElement = {
  "1.0": (tag, props, ...children) => React.createElement(tag, props && transformProps(props), ...children),
};

export const createFragment = {
  "1.0": (...children) => React.createElement(React.Fragment, null, ...children),
};

export const mount = {
  "1.0": (element, id) => {
    const root = ReactDOM.createRoot(document.getElementById(id));
    root.render(element);

    return () => root.unmount();
  },
}
