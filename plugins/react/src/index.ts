import * as React from 'react';
import * as ReactDOM from 'react-dom';

export const main = {
  createElement: React.createElement,
  render(app: JSX.Element, id: string): void {
    // tslint:disable-next-line:no-expression-statement
    ReactDOM.render(app, document.getElementById(id));
  },

  // @hidden
  withState(
    createState: (update: () => void) => { readonly get: () => any },
    component: React.ComponentType
  ): React.ComponentType {
    class State<T extends { readonly $$_state?: any }> extends React.Component<
      T
    > {
      // tslint:disable-next-line:variable-name
      public readonly _state = createState(this.forceUpdate.bind(this));

      public render(): JSX.Element {
        return React.createElement(component, {
          ...this.props,

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
