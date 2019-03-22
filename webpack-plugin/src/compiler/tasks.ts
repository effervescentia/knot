import { ModuleStatus, ServerStatus } from '../types';
import fetch from './fetch';

const MAX_ATTEMPTS = 10;
const INFINITE_ATTEMPTS = 100;
const ATTEMPT_TIMEOUT = 1000;

export function awaitServerIdle({ baseUrl }): () => Promise<void> {
  return () => new Promise(awaitStatus(ServerStatus.IDLE, baseUrl));
}

export function awaitComilationComplete({ baseUrl }): () => Promise<void> {
  return () =>
    new Promise(awaitStatus(ServerStatus.COMPLETE, baseUrl, INFINITE_ATTEMPTS));
}

export function awaitModuleComplete({
  baseUrl
}): (path: string) => Promise<void> {
  return path => new Promise(awaitModuleStatus(path, baseUrl));
}

export function addModule({
  baseUrl
}): (path: string) => Promise<void | Response> {
  return path =>
    fetch(`${baseUrl}/module`, {
      body: path,
      method: 'PUT'
    }).catch(handleError('failed to add module to knot compilation context'));
}

export function generateModule({
  baseUrl
}): (path: string) => Promise<void | string> {
  return path =>
    fetch(`${baseUrl}/module`, {
      body: path,
      method: 'POST'
    })
      .then(res => res.text())
      .catch(handleError('unable to get module from compiler'));
}

export function invalidateModule({
  baseUrl
}): (path: string) => Promise<void | Response> {
  return path =>
    fetch(`${baseUrl}/module`, {
      body: path,
      method: 'DELETE'
    }).catch(handleError('unable to invalidate module'));
}

export function killServer({ baseUrl }): () => Promise<void | Response> {
  return () =>
    fetch(`${baseUrl}/kill`, {
      method: 'POST'
    }).catch(() => null);
}

function awaitModuleStatus(
  path: string,
  baseUrl: string
): (resolve: () => void, reject: (err: any) => void) => void {
  return awaitPromise(
    (resolve, reject) =>
      fetch(`${baseUrl}/module/status`, {
        body: path,
        method: 'POST'
      })
        .then(res => res.text())
        .then(status =>
          status === ModuleStatus.COMPLETE ? resolve() : reject()
        ),
    INFINITE_ATTEMPTS
  );
}

function awaitStatus(
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

// tslint:disable:no-expression-statement
function awaitPromise(
  createPromise: (resolve: () => void, reject: () => void) => Promise<void>,
  maxAttempts: number
): (resolve: () => void, reject: (err: any) => void) => void {
  const errorMsg = 'knot compiler did not return a healthy response';
  // tslint:disable-next-line:no-let
  let attempts = 0;

  return function tryMatchStatus(resolve, reject): void {
    setTimeout(() => {
      attempts += 1;

      function handleFailure(): void {
        if (maxAttempts !== 0 && attempts === maxAttempts) {
          reject(errorMsg);
        } else {
          tryMatchStatus(resolve, reject);
        }
      }

      createPromise(resolve, handleFailure).catch(handleFailure);
    }, ATTEMPT_TIMEOUT);
  };
}

function handleError(errMsg: string): (e: Error) => void {
  return e => {
    // tslint:disable-next-line:no-console
    console.error(errMsg, e);

    throw new Error(errMsg);
  };
}
