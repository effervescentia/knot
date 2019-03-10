import baseFetch from 'cross-fetch';
import { ArgumentTypes } from '../types';

export default async function fetch(...args: ArgumentTypes<typeof baseFetch>) {
  const res = await baseFetch(...args);

  if (res.status >= 400) {
    throw new Error('Bad response from server');
  }

  return res;
}
