import { ModuleStatus, ServerStatus } from '../types';
import { ATTEMPT_TIMEOUT, INFINITE_ATTEMPTS, MAX_ATTEMPTS } from './constants';
import fetch from './fetch';

export function awaitPromise(
  createPromise: (
    resolve: () => void,
    reject: (err?: Error) => void,
    fail: (err?: Error) => void
  ) => Promise<void>,
  maxAttempts: number,
  timeout = ATTEMPT_TIMEOUT
): (resolve: () => void, reject: (err: any) => void) => void {
  const errorMsg = 'knot compiler did not return a healthy response';
  let attempts = 0;

  return function tryMatchStatus(resolve, reject): void {
    setTimeout(() => {
      attempts += 1;

      function retry(): void {
        if (maxAttempts !== 0 && attempts === maxAttempts) {
          reject(errorMsg);
        } else {
          tryMatchStatus(resolve, reject);
        }
      }

      createPromise(resolve, retry, reject).catch(retry);
    }, timeout);
  };
}

export function awaitModuleStatus(
  path: string,
  baseUrl: string
): (resolve: () => void, reject: (err: any) => void) => void {
  return awaitPromise(
    (resolve, reject, fail) =>
      fetch(`${baseUrl}/module/status`, {
        body: path,
        method: 'POST'
      }).then(res =>
        res.text().then(status => {
          switch (status) {
            case ModuleStatus.COMPLETE:
              return resolve();
            case ModuleStatus.FAILED:
              return fail(new Error(`module "${path}" failed to compile`));
            default:
              return reject();
          }
        })
      ),
    INFINITE_ATTEMPTS,
    50
  );
}

export function awaitStatus(
  statusType: ServerStatus,
  baseUrl: string,
  maxAttempts = MAX_ATTEMPTS
): (resolve: () => void, reject: (err: any) => void) => void {
  return awaitPromise(
    (resolve, reject) =>
      fetch(`${baseUrl}/status`)
        .then(res => res.text())
        .then(status => (status === statusType ? resolve() : reject())),
    maxAttempts
  );
}

export function handleError(errMsg: string): (e: Error) => void {
  return e => {
    console.error(errMsg, e);

    throw new Error(errMsg);
  };
}
