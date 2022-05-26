import test from 'ava';
import fs from 'fs-extra';
import os from 'os';
import path from 'path';

import { Target } from '../../types';
import { CompilerErrorType, ModuleStatus, Status } from '../protocol';
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

const runWithClient =
  (overrides: Partial<ClientOptions> = {}) =>
  async (
    test: (client: Client, resolve: (file: string) => string) => Promise<any>
  ) => {
    const options = {
      knotc: LOCAL_BINARY,
      cwd: JS_TARGET_FIXTURE,
      ...overrides,
    };
    const client = new Client(options);

    try {
      await test(client, (file) => path.join(options.cwd, 'src', file));
    } finally {
      client.terminate();
    }
  };

test('starts with idle status', (t) =>
  runWithClient()(async (client) => {
    const { status } = await client.status();

    t.is(status, Status.IDLE);
  }));

test('processes a module', (t) =>
  runWithClient()(async (client, resolve) => {
    await client.addModule({ path: resolve(MAIN_MODULE) });

    const { status } = await client.status();

    t.is(status, Status.IDLE);

    const { status: moduleStatus } = await client.moduleStatus({
      path: resolve(MAIN_MODULE),
    });

    t.is(moduleStatus, ModuleStatus.VALID);

    const { data } = await client.fetchModule({ path: resolve(MAIN_MODULE) });

    t.is(data, Fixture.SIMPLE_MAIN);
  }));

test('processes multiple modules', (t) =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await client.addModule({ path: resolve(MAIN_MODULE) });

    const [{ data: mainData }, { data: appData }, { data: constData }] =
      await Promise.all([
        client.fetchModule({ path: resolve(MAIN_MODULE) }),
        client.fetchModule({ path: resolve(APP_MODULE) }),
        client.fetchModule({ path: resolve(CONSTANTS_MODULE) }),
      ]);

    t.is(mainData, Fixture.COMPLEX_MAIN);
    t.is(appData, Fixture.COMPLEX_APP);
    t.is(constData, Fixture.COMPLEX_CONSTANTS);
  }));

test('processes modules incrementally', (t) =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await client.addModule({ path: resolve(APP_MODULE) });

    const { data: appData } = await client.fetchModule({
      path: resolve(APP_MODULE),
    });

    t.is(appData, Fixture.COMPLEX_APP);

    const { status: moduleStatus } = await client.moduleStatus({
      path: resolve(MAIN_MODULE),
    });

    t.is(moduleStatus, ModuleStatus.NONE);

    await client.addModule({ path: resolve(MAIN_MODULE) });

    const [{ data: mainData }, { data: constData }] = await Promise.all([
      client.fetchModule({ path: resolve(MAIN_MODULE) }),
      client.fetchModule({ path: resolve(CONSTANTS_MODULE) }),
    ]);

    t.is(mainData, Fixture.COMPLEX_MAIN);
    t.is(constData, Fixture.COMPLEX_CONSTANTS);
  }));

test('purges root module safely', (t) =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await client.addModule({ path: resolve(MAIN_MODULE) });
    await client.removeModule({ path: resolve(MAIN_MODULE) });

    const [{ status }, { status: mainStatus }, { status: appStatus }] =
      await Promise.all([
        client.status(),
        client.moduleStatus({ path: resolve(MAIN_MODULE) }),
        client.moduleStatus({ path: resolve(APP_MODULE) }),
      ]);

    t.is(status, Status.IDLE);
    t.is(mainStatus, ModuleStatus.PURGED);
    t.is(appStatus, ModuleStatus.VALID);
  }));

test('purges leaf module and reprocesses', (t) =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    const errorPromise = new Promise((resolve) => {
      client.replaceErrorHandler((error) => {
        client.removeErrorHandler();
        resolve(error);
      });
    });

    await client.addModule({ path: resolve(MAIN_MODULE) });
    await client.removeModule({ path: resolve(APP_MODULE) });

    const [{ status }, { status: mainStatus }, { status: appStatus }] =
      await Promise.all([
        client.status(),
        client.moduleStatus({ path: resolve(MAIN_MODULE) }),
        client.moduleStatus({ path: resolve(APP_MODULE) }),
      ]);

    t.is(status, Status.IDLE);
    t.is(mainStatus, ModuleStatus.VALID);
    t.is(appStatus, ModuleStatus.PURGED);

    const error = await errorPromise;
    t.deepEqual(error, {
      type: CompilerErrorType.UNRESOLVED_MODULE,
      message: "cannot resolve module '@/App'",
    });
  }));

test('resets compilation context', (t) =>
  runWithClient({
    cwd: COMPLEX_FIXTURE,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await client.addModule({ path: resolve(MAIN_MODULE) });
    await client.reset();

    const [
      { status },
      { status: mainStatus },
      { status: appStatus },
      { status: constStatus },
    ] = await Promise.all([
      client.status(),
      client.moduleStatus({ path: resolve(MAIN_MODULE) }),
      client.moduleStatus({ path: resolve(APP_MODULE) }),
      client.moduleStatus({ path: resolve(CONSTANTS_MODULE) }),
    ]);

    t.is(status, Status.IDLE);
    t.is(mainStatus, ModuleStatus.NONE);
    t.is(appStatus, ModuleStatus.NONE);
    t.is(constStatus, ModuleStatus.NONE);
  }));

test('updates module incrementally', async (t) => {
  const barModule = 'bar.kn';
  const cwd = await createFixture(COMPLEX_FIXTURE);

  await fs.promises.writeFile(
    path.join(cwd, 'src', barModule),
    'const BAR = 123;',
    'utf8'
  );

  await runWithClient({
    cwd,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await Promise.all([
      client.addModule({ path: resolve(MAIN_MODULE) }),
      client.addModule({ path: resolve(barModule) }),
    ]);

    const { data: barData } = await client.fetchModule({
      path: resolve(barModule),
    });

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
      'utf8'
    );

    await client.updateModule({ path: resolve(CONSTANTS_MODULE) });

    const [{ status }, { data: nextConstantsData }] = await Promise.all([
      client.status(),
      client.fetchModule({ path: resolve(CONSTANTS_MODULE) }),
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

test('recovers from error state', async (t) => {
  const cwd = await createFixture(COMPLEX_FIXTURE);

  await fs.promises.writeFile(
    path.join(cwd, 'src', CONSTANTS_MODULE),
    'main const const = "foo";',
    'utf8'
  );

  await runWithClient({
    cwd,
    target: Target.JAVASCRIPT_ES6,
  })(async (client, resolve) => {
    await client.addModule({ path: resolve(MAIN_MODULE) });

    const [{ status: mainStatus }, { status: constantsStatus }] =
      await Promise.all([
        client.moduleStatus({ path: resolve(MAIN_MODULE) }),
        client.moduleStatus({ path: resolve(CONSTANTS_MODULE) }),
      ]);

    t.is(mainStatus, ModuleStatus.PARTIAL);
    t.is(constantsStatus, ModuleStatus.PARTIAL);

    await fs.promises.writeFile(
      path.join(cwd, 'src', CONSTANTS_MODULE),
      `main const TIMEOUT = 100;`,
      'utf8'
    );

    await client.updateModule({ path: resolve(CONSTANTS_MODULE) });

    const [
      { status },
      { status: nextMainStatus },
      { status: nextConstantsStatus },
      { data: nextConstantsData },
    ] = await Promise.all([
      client.status(),
      client.moduleStatus({ path: resolve(MAIN_MODULE) }),
      client.moduleStatus({ path: resolve(CONSTANTS_MODULE) }),
      client.fetchModule({ path: resolve(CONSTANTS_MODULE) }),
    ]);

    t.is(status, Status.IDLE);
    t.is(nextMainStatus, ModuleStatus.VALID);
    t.is(nextConstantsStatus, ModuleStatus.VALID);
    t.is(nextConstantsData, Fixture.COMPLEX_CONSTANTS);
  });
});
