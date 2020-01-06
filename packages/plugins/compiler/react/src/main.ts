import * as React from 'react';
import * as ReactDOM from 'react-dom';

export const createElement = React.createElement;

export function createFragment(
  ...children: Readonly<JSX.Element[]>
): ReturnType<typeof React.createElement> {
  return React.createElement(React.Fragment, null, ...children);
}

export function render(app: JSX.Element, id: string): void {
  ReactDOM.render(app, document.getElementById(id));
}

// @hidden
export function withState(
  createState: (update: () => void) => { readonly get: () => any },
  component: React.ComponentType
): React.ComponentClass {
  class State<T extends { readonly $$_state?: any }> extends React.Component<
    T
  > {
    public readonly _state = createState(this.forceUpdate.bind(this));

    public render(): JSX.Element {
      return createElement(component, {
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
