// import { JSXPlugin, PropsType, STATE_MAP_KEY } from '@knot/plugin-utils';
import { JSXPlugin, PropsType, Style } from '@knot/plugin-utils';
import * as React from 'react';
import * as ReactDOM from 'react-dom';

// import StateFactory from './state';

const Plugin: JSXPlugin<React.ComponentType, JSX.Element> = {
  // createComponent: (_name, component) => component as any,

  // createElement: React.createElement,
  createTag: React.createElement,

  createFragment(...children) {
    return React.createElement(React.Fragment, null, ...children);
  },

  render(app, id) {
    return ReactDOM.render(app, document.getElementById(id));
  },

  bindStyle<P extends PropsType>(
    component: ((props: P) => JSX.Element) | string,
    style: Style
  ): (props: P) => JSX.Element {
    return (props: P) =>
      React.createElement(component as any, {
        ...props,
        className:
          (props.className ? `${props.className} ` : '') + style.getClass(),
      });
  },

  // withState: (createState, component) => {
  //   class State<P extends PropsType> extends React.Component<P> {
  //     public readonly _state = createState(
  //       new StateFactory(this.forceUpdate.bind(this))
  //     );

  //     public render(): JSX.Element {
  //       return React.createElement(component, {
  //         ...this.props,

  //         [STATE_MAP_KEY]: {
  //           ...this.props[STATE_MAP_KEY],
  //           ...this._state.get(),
  //         },
  //       } as any);
  //     }
  //   }

  //   return State;
  // },
};

export default Plugin;
