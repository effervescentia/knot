/* eslint-disable @typescript-eslint/camelcase */
import { JSXPlugin, PluginError } from '@knot/plugin-utils';
import Vue from 'vue';

import StateFactory from './state';

type ElementFactory = (
  tagName: string,
  props?: object,
  children?: Vue.VNode[]
) => Vue.VNode;

type VueElement = (createElement: ElementFactory) => Vue.VNode;

type VueComponent = (props: { $$_state?: any }) => VueElement;

const Plugin: JSXPlugin<string | VueComponent, VueElement> = {
  createElement: (element, rawProps, ...children) => createElement => {
    const props = rawProps || {};

    if (typeof element === 'function') {
      return element(props)(createElement);
    }

    const attrs = Object.entries(props)
      .filter(([, value]) => typeof value !== 'function')
      .reduce((acc, [key, value]) => Object.assign(acc, { [key]: value }), {});

    const eventHandlers = Object.keys(props)
      .filter(key => {
        console.log('prop', key);
        return key.startsWith('on') && typeof props[key] === 'function';
      })
      .reduce(
        (acc, key) =>
          Object.assign(acc, { [key.substr(2).toLowerCase()]: props[key] }),
        {}
      );

    console.log(eventHandlers);

    return createElement(
      element,
      {
        attrs,
        on: eventHandlers
      },
      children.map(child => {
        if (typeof child === 'function') {
          return child(createElement);
        }

        return child;
      })
    );
  },

  createFragment: _ => {
    throw new PluginError('Vue.js does not support fragments');
  },

  render: (app, id) =>
    new Vue({
      el: `#${id}`,
      render(createElement) {
        return app(createElement);
      }
    }),

  withState: (createState, component: VueComponent) => {
    const observable = Vue.observable({});
    const state = createState(new StateFactory(observable));

    return props => createElement =>
      component({
        ...props,
        $$_state: {
          ...props?.$$_state,
          ...state.get()
        }
      })(createElement);
  }
};

export default Plugin;
