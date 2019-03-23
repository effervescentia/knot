import baseFetch from 'cross-fetch';
import { ArgumentTypes } from '../types';

export class NetworkError extends Error {
  constructor(msg: string, public statusCode: number) {
    // tslint:disable-next-line:no-expression-statement
    super(msg);
  }
}

export default async function fetch(
  ...args: ArgumentTypes<typeof baseFetch>
): ReturnType<typeof baseFetch> {
  const res = await baseFetch(...args);

  if (res.status >= 400) {
    throw new NetworkError('Bad response from server', res.status);
  }

  return res;
}
