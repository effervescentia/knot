import { PropsType } from '@knot/plugin-utils';

import { ElementFactory, VueElement } from '../types';
import createNodeData from './data';

const createElement = (
  factory: ElementFactory,
  tagName: string,
  props: PropsType,
  children: VueElement[]
) => {
  const data = createNodeData(props);

  return factory(
    tagName,
    data,
    children.map(child => {
      if (typeof child === 'function') {
        return child(factory);
      }

      return child;
    })
  );
};

export default createElement;
