import fetch from './fetch';

const MAX_ATTEMPTS = 10;
const ATTEMPT_TIMEOUT = 1000;

export function awaitServerIdle({ baseUrl }) {
  return () => new Promise(awaitStatus('idle', baseUrl));
}

export function awaitComilationComplete({ baseUrl }) {
  return () => new Promise(awaitStatus('complete', baseUrl, 0));
}

export function addModule({ baseUrl }) {
  return (path: string) => {
    console.log(`add '${path}' to knot compiler`);

    return fetch(`${baseUrl}/module`, {
      method: 'PUT',
      body: path
    }).catch(handleError('failed to add module to knot compilation context'));
  };
}

export function generateModule({ baseUrl }) {
  return (path: string) =>
    fetch(`${baseUrl}/module`, {
      method: 'POST',
      body: path
    })
      .then(res => res.text())
      .catch(handleError('unable to get module from compiler'));
}

export function resetContext({ baseUrl }) {
  return () =>
    fetch(`${baseUrl}/context`, {
      method: 'DELETE'
    }).catch(handleError('unable to reset compiler context'));
}

export function killServer({ baseUrl }) {
  return () =>
    fetch(`${baseUrl}/kill`, {
      method: 'POST'
    }).catch(() => null);
}

function awaitStatus(statusType, baseUrl: string, maxAttempts = MAX_ATTEMPTS) {
  const errorMsg = 'knot compiler did not return a healthy response';
  let attempts = 0;

  return function tryMatchStatus(resolve, reject) {
    return setTimeout(() => {
      attempts += 1;

      function handleFailure() {
        if (maxAttempts !== 0 && attempts === maxAttempts) {
          reject(errorMsg);
        } else {
          tryMatchStatus(resolve, reject);
        }
      }

      fetch(`${baseUrl}/status`)
        .then(res => res.text())
        .then(status => (status === statusType ? resolve() : handleFailure()))
        .catch(handleFailure);
    }, ATTEMPT_TIMEOUT);
  };
}

function handleError(errMsg: string) {
  return e => {
    console.error(errMsg, e);

    throw new Error(errMsg);
  };
}
