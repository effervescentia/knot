import test from 'ava';
import fs from 'fs-extra';
import os from 'os';
import path from 'path';

import { Target } from '../../types';
import { ModuleStatus, Status } from '../protocol';
import Client, { ClientOptions } from '.';
import * as Fixture from './index.fixture';

const COMPILER_DIR = path.resolve(__dirname, '../../../../../../../compiler');
const FIXTURE_DIR = path.join(COMPILER_DIR, 'test/executable/.fixtures');

const JS_TARGET_FIXTURE = path.join(FIXTURE_DIR, 'js_target');
const COMPLEX_FIXTURE = path.join(FIXTURE_DIR, 'complex');

const LOCAL_BINARY = path.join(
  COMPILER_DIR,
  '_esy/default/build/install/default/bin/knotc.exe'
);

const MAIN_MODULE = 'main.kn';
const APP_MODULE = 'App.kn';
const CONSTANTS_MODULE = 'common/constants.kn';

const createFixture = async (source: string) => {
  const fixture = await fs.promises.mkdtemp(path.join(os.tmpdir(), 'fixture'));

  await fs.copy(source, fixture, { recursive: true });

  return fixture;
};

const runWithClient = (options: Partial<ClientOptions> = {}) => async (
  test: (client: Client) => Promise<any>
) => {
  const client = new Client({
    knotc: LOCAL_BINARY,
    cwd: JS_TARGET_FIXTURE,
    ...options
  });

  try {
    await test(client);
  } finally {
    client.terminate();
  }
};

test('starts with idle status', t =>
  runWithClient()(async client => {
    const { status } = await client.status();

    t.is(status, Status.IDLE);
  }));

test('processes a module', t =>
  runWithClient()(async client => {
    await client.addModule({ path: MAIN_MODULE });

    const { status } = await client.status();

    t.is(status, Status.IDLE);

    const { status: moduleStatus } = await client.moduleStatus({
      path: MAIN_MODULE
    });

    t.is(moduleStatus, ModuleStatus.VALID);

    const { data } = await client.fetchModule({ path: MAIN_MODULE });

    t.is(data, Fixture.SIMPLE_MAIN);
  }));

test('processes multiple modules', t =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: MAIN_MODULE });

    const [
      { data: mainData },
      { data: appData },
      { data: constData }
    ] = await Promise.all([
      client.fetchModule({ path: MAIN_MODULE }),
      client.fetchModule({ path: APP_MODULE }),
      client.fetchModule({ path: CONSTANTS_MODULE })
    ]);

    t.is(mainData, Fixture.COMPLEX_MAIN);
    t.is(appData, Fixture.COMPLEX_APP);
    t.is(constData, Fixture.COMPLEX_CONSTANTS);
  }));

test('processes modules incrementally', t =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: APP_MODULE });

    const { data: appData } = await client.fetchModule({ path: APP_MODULE });

    t.is(appData, Fixture.COMPLEX_APP);

    const { status: moduleStatus } = await client.moduleStatus({
      path: MAIN_MODULE
    });

    t.is(moduleStatus, ModuleStatus.NONE);

    await client.addModule({ path: MAIN_MODULE });

    const [{ data: mainData }, { data: constData }] = await Promise.all([
      client.fetchModule({ path: MAIN_MODULE }),
      client.fetchModule({ path: CONSTANTS_MODULE })
    ]);

    t.is(mainData, Fixture.COMPLEX_MAIN);
    t.is(constData, Fixture.COMPLEX_CONSTANTS);
  }));

test('purges root module safely', t =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: MAIN_MODULE });
    await client.removeModule({ path: MAIN_MODULE });

    const [
      { status },
      { status: mainStatus },
      { status: appStatus }
    ] = await Promise.all([
      client.status(),
      client.moduleStatus({ path: MAIN_MODULE }),
      client.moduleStatus({ path: APP_MODULE })
    ]);

    t.is(status, Status.IDLE);
    t.is(mainStatus, ModuleStatus.NONE);
    t.is(appStatus, ModuleStatus.VALID);
  }));

test('purges leaf module and reprocesses', t =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: MAIN_MODULE });
    await client.removeModule({ path: APP_MODULE });

    const [
      { status },
      // { status: mainStatus },
      { status: appStatus }
    ] = await Promise.all([
      client.status(),
      // client.moduleStatus({ path: MAIN_MODULE }),
      client.moduleStatus({ path: APP_MODULE })
    ]);

    t.is(status, Status.IDLE);
    // FIXME: this module should now have an error
    // t.is(mainStatus, ModuleStatus.ERROR);
    t.is(appStatus, ModuleStatus.NONE);
  }));

test('resets compilation context', t =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: MAIN_MODULE });
    await client.reset();

    const [
      { status },
      { status: mainStatus },
      { status: appStatus },
      { status: constStatus }
    ] = await Promise.all([
      client.status(),
      client.moduleStatus({ path: MAIN_MODULE }),
      client.moduleStatus({ path: APP_MODULE }),
      client.moduleStatus({ path: CONSTANTS_MODULE })
    ]);

    t.is(status, Status.IDLE);
    t.is(mainStatus, ModuleStatus.NONE);
    t.is(appStatus, ModuleStatus.NONE);
    t.is(constStatus, ModuleStatus.NONE);
  }));

test('updates module incrementally', async t => {
  const barModule = 'bar.kn';
  const cwd = await createFixture(COMPLEX_FIXTURE);

  await fs.promises.writeFile(
    path.join(cwd, 'src', barModule),
    'const BAR = 123;',
    'utf-8'
  );

  await runWithClient({
    cwd,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await Promise.all([
      client.addModule({ path: MAIN_MODULE }),
      client.addModule({ path: barModule })
    ]);

    const { data: barData } = await client.fetchModule({ path: barModule });

    t.is(
      barData,
      `import $knot from "@knot/runtime";
var BAR = 123;
export { BAR };
`
    );

    await fs.promises.writeFile(
      path.join(cwd, 'src', CONSTANTS_MODULE),
      `import { BAR } from "@/bar";

main const TIMEOUT = 100;
`,
      'utf-8'
    );

    await client.updateModule({ path: CONSTANTS_MODULE });

    const [{ status }, { data: nextConstantsData }] = await Promise.all([
      client.status(),
      client.fetchModule({ path: CONSTANTS_MODULE })
    ]);

    t.is(status, Status.IDLE);
    t.is(
      nextConstantsData,
      `import $knot from "@knot/runtime";
import { BAR } from "./bar";
var TIMEOUT = 100;
export { TIMEOUT };
export { TIMEOUT as main };
`
    );
  });
});

test('recovers from error state', async t => {
  const cwd = await createFixture(COMPLEX_FIXTURE);

  await fs.promises.writeFile(
    path.join(cwd, 'src', CONSTANTS_MODULE),
    'main const const = 100;',
    'utf-8'
  );

  await runWithClient({
    cwd,
    target: Target.JAVASCRIPT_ES6
  })(async client => {
    await client.addModule({ path: MAIN_MODULE });

    // const { status: mainStatus } = await client.moduleStatus({
    //   path: MAIN_MODULE
    // });

    // t.is(mainStatus, ModuleStatus.PENDING);

    await fs.promises.writeFile(
      path.join(cwd, 'src', CONSTANTS_MODULE),
      `main const TIMEOUT = 100;`,
      'utf-8'
    );

    await client.updateModule({ path: CONSTANTS_MODULE });

    const [{ status }, { data: nextConstantsData }] = await Promise.all([
      client.status(),
      client.fetchModule({ path: CONSTANTS_MODULE })
    ]);

    t.is(status, Status.IDLE);
    t.is(nextConstantsData, Fixture.COMPLEX_CONSTANTS);
  });
});
