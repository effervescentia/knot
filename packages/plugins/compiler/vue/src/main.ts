/* eslint-disable @typescript-eslint/camelcase */
import { JSXPlugin, PluginError } from '@knot/plugin-utils';
import Vue from 'vue';

import createData from './data';
// import StateFactory from './state';
import { ElementFactory, VueComponent, VueElement } from './types';

const wrapFactory = (factory: ElementFactory) => {
  const render = (element: VueElement | string) => {
    console.log('rendering', element);

    if (typeof element === 'string') {
      return element;
    }

    const data = createData(element.props);
    const children = element.children.map(render);

    console.log('with data', data);

    return factory(element.type, data, children);
  };

  return render;
};

const Plugin: JSXPlugin<VueComponent, VueElement> = {
  createComponent: (_name, component) => ({
    inheritAttrs: false,
    render(factory) {
      const element = component(this.$attrs);
      const render = wrapFactory(factory);

      return render(element);
    }
  }),

  createElement: (element, props, ...children) => ({
    type: element,
    props: props || {},
    children
  }),

  createFragment: () => {
    throw new PluginError('Vue.js does not yet support fragments');
  },

  render: (app, id) =>
    new Vue({
      el: `#${id}`,
      render(factory) {
        const render = wrapFactory(factory);

        return render(app);
      }
    }),

  withState: (createState, component) => {
    // const observable = Vue.observable({});
    // const state = createState(new StateFactory(observable));

    console.warn('withState', createState, component);

    return {
      ...component
    };
  }
};

export default Plugin;
