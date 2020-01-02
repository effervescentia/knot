import { ServerStatus } from '../types';
import { INFINITE_ATTEMPTS } from './constants';
import fetch from './fetch';
import { awaitModuleStatus, awaitStatus, handleError } from './utils';

export function awaitServerIdle({ baseUrl }): Promise<void> {
  return new Promise(awaitStatus(ServerStatus.IDLE, baseUrl));
}

export function awaitCompilationComplete({ baseUrl }): Promise<void> {
  return new Promise(
    awaitStatus(ServerStatus.COMPLETE, baseUrl, INFINITE_ATTEMPTS)
  );
}

export function awaitModuleComplete({ baseUrl }, path: string): Promise<void> {
  return new Promise(awaitModuleStatus(path, baseUrl));
}

export function addModule({ baseUrl }, path: string): Promise<void | Response> {
  return fetch(`${baseUrl}/module`, {
    body: path,
    method: 'PUT'
  }).catch(handleError('failed to add module to knot compilation context'));
}

export function generateModule(
  { baseUrl },
  path: string
): Promise<void | string> {
  return fetch(`${baseUrl}/module`, {
    body: path,
    method: 'POST'
  })
    .then(res => res.text())
    .catch(handleError('unable to get module from compiler'));
}

export function invalidateModule(
  { baseUrl },
  path: string
): Promise<void | Response> {
  return fetch(`${baseUrl}/module`, {
    body: path,
    method: 'DELETE'
  }).catch(handleError('unable to invalidate module'));
}

export async function killServer({ baseUrl }): Promise<void | Response> {
  try {
    return fetch(`${baseUrl}/kill`, {
      method: 'POST'
    });
  } catch (e) {
    return null;
  }
}
