import baseFetch from 'cross-fetch';

type ArgumentTypes<F> = F extends (...args: infer A) => any ? A : never;

export class NetworkError extends Error {
  constructor(msg: string, public statusCode: number) {
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
