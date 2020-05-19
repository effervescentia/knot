import { StateFactory } from '@knot/plugin-utils';

class ReactStateFactory<S extends object> implements StateFactory<S> {
  private properties = [];
  private mutators = [];

  constructor(private forceUpdate: () => void) {}

  prop<K extends keyof S>(name: K, property: S[K]) {
    this.properties.push(name);

    return property;
  }

  mut<K extends keyof S, T extends S[K] & ((...args: any[]) => void)>(
    name: K,
    mutator: T
  ) {
    this.mutators.push(name);

    return ((...args) => {
      mutator(...args);
      this.forceUpdate();
    }) as T;
  }

  build(state: S) {
    return state;
  }
}

export default ReactStateFactory;
