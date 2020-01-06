import execa from 'execa';
import fs from 'fs-extra';
import path from 'path';

const IGNORED_ARTIFACTS: readonly string[] = ['package.json', 'README.md'];

export default async function installPlatformArtifacts(
  rootDir: string
): Promise<void> {
  const entrypoint = path.join(rootDir, 'esyInstallRelease.js');
  const platformsDir = path.join(rootDir, 'platforms');

  try {
    await fs.access(entrypoint, fs.constants.F_OK);
    console.log('binary already installed');
    return;
  } catch (e) {
    // install binaries if they do not exist
  }

  const copyArtifacts = (platform: string): Promise<void> =>
    fs.copy(path.join(platformsDir, platform), rootDir, {
      filter: src => !IGNORED_ARTIFACTS.includes(path.basename(src))
    });

  switch (process.platform) {
    case 'darwin':
      await copyArtifacts('macos');
      break;
    case 'linux':
      await copyArtifacts('linux');
      break;
    default:
      console.error(
        'This platform (' + process.platform + ') is not supported yet.'
      );
      process.exit(-1);
  }

  // TODO: enable when confident
  // await fs.remove(PLATFORMS_DIR);

  const { stdout } = await execa('node', [entrypoint], {
    env: {
      ESY_RELEASE_REWRITE_PREFIX: 'true'
    }
  });
  console.log(stdout);
}
