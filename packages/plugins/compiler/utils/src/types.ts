export interface StateFactory<S extends object = {}> {
  prop<K extends keyof S>(name: K, property: S[K]): S[K];

  mut<K extends keyof S, T extends S[K] & ((value: any) => void)>(
    name: K,
    mutator: T
  ): T;

  build(state: S): S;
}

export interface JSXPlugin<T, R> {
  createElement(
    element: string | T,
    props?: object | null,
    ...children: R[]
  ): R;

  withState(
    createState: (factory: StateFactory) => { readonly get: () => any },
    component: T
  ): T;

  createFragment(...children: R[]): R;

  render(app: R, id: string): void;
}
