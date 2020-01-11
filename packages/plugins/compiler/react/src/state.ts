import { StateFactory } from '@knot/plugin-utils';

class ReactStateFactory<S extends object> implements StateFactory<S> {
  private properties = [];
  private mutators = [];

  constructor(private forceUpdate: () => void) {}

  prop<K extends keyof S>(name: K, property: S[K]) {
    this.properties.push(name);

    return property;
  }

  mut<K extends keyof S, T extends S[K] & ((value: any) => void)>(
    name: K,
    mutator: T
  ) {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const self = this;

    this.mutators.push(name);

    return function() {
      // eslint-disable-next-line prefer-spread, prefer-rest-params
      mutator.apply(null, arguments);
      self.forceUpdate();
    } as any;
  }

  build(state: S) {
    return state;
  }
}

export default ReactStateFactory;
