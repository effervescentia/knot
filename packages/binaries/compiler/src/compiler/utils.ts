import { ATTEMPT_TIMEOUT, INFINITE_ATTEMPTS } from './constants';

export function pollingPromise(
  createPromise: (
    resolve: () => void,
    reject: (err?: Error) => void,
    abort: (err?: Error) => void
  ) => Promise<void>,
  maxAttempts = INFINITE_ATTEMPTS,
  timeout = ATTEMPT_TIMEOUT
): Promise<void> {
  let attempts = 0;

  function handler(resolve: () => void, reject: (err: any) => void): void {
    function retry(err: unknown): void {
      if (maxAttempts !== 0 && attempts === maxAttempts) {
        reject(`failed after ${attempts} attempts: ${err}`);
      } else {
        console.warn(`promise failed, retrying in ${timeout}ms: ${err}`);
        setTimeout(() => handler(resolve, reject), timeout);
      }
    }

    attempts++;

    createPromise(resolve, retry, reject).catch(retry);
  }

  return new Promise(handler);
}
