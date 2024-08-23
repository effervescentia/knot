import { $knot } from '@knot/runtime';

export function mount(element, id) {
  return $knot.plugin.get('view', 'mount', '1.0')(element, id);
}
