import { JSXPlugin } from '@knot/plugin-utils';
import * as React from 'react';
import * as ReactDOM from 'react-dom';

import StateFactory from './state';

const Plugin: JSXPlugin<React.ComponentType, JSX.Element> = {
  createComponent: (_name, component) => component as any,

  createElement: React.createElement,

  createFragment: (...children) =>
    React.createElement(React.Fragment, null, ...children),

  render: (app, id) => ReactDOM.render(app, document.getElementById(id)),

  withState: (createState, component) => {
    class State<T extends { readonly $$_state?: any }> extends React.Component<
      T
    > {
      public readonly _state = createState(
        new StateFactory(this.forceUpdate.bind(this))
      );

      public render(): JSX.Element {
        return React.createElement(component, {
          ...this.props,

          // eslint-disable-next-line @typescript-eslint/camelcase
          $$_state: {
            ...this.props.$$_state,
            ...this._state.get()
          }
        } as any);
      }
    }

    return State;
  }
};

export default Plugin;
