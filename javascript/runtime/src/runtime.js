export class Runtime {
  static pluginKey(namespace, property, version) {
    return `${namespace}|${property}|${version}`;
  }

  plugins = new Map();

  plugin = {
    get: (namespace, property, version) => this.plugins.get(Runtime.pluginKey(namespace, property, version)),

    register: (namespace, module) => Object
      .entries(module)
      .forEach(([property, versions]) => Object
        .entries(versions)
        .forEach(([version, value]) => this.plugins.set(Runtime.pluginKey(namespace, property, version), value))
      ),
  };
}
