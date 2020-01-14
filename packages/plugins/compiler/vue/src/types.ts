import { PropsType } from '@knot/plugin-utils';
import { VNodeData } from 'vue';

export type ElementFactory = (
  tagName: string,
  data?: VNodeData,
  children?: Vue.VNode[]
) => Vue.VNode;

export type VueElement = (createElement: ElementFactory) => Vue.VNode;

export type VueComponent = (props: PropsType) => VueElement;
