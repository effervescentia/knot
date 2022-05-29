// import { JSXPlugin, PropsType, STATE_MAP_KEY } from '@knot/plugin-utils';
import { JSXPlugin } from '@knot/plugin-utils';
import * as React from 'react';
import * as ReactDOM from 'react-dom';

// import StateFactory from './state';

const Plugin: JSXPlugin<React.ComponentType, JSX.Element> = {
  // createComponent: (_name, component) => component as any,

  // createElement: React.createElement,
  createTag: React.createElement,

  createFragment: (...children) =>
    React.createElement(React.Fragment, null, ...children),

  render: (app, id) => ReactDOM.render(app, document.getElementById(id)),

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
