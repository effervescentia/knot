#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const execa = require('execa');

const ENTRYPOINT = path.join(__dirname, 'esyInstallRelease.js');
const PLATFORMS_DIR = path.join(__dirname, 'platforms')
const IGNORED_ARTIFACTS = ['package.json', 'README.md'];

const copyArtifacts = platform => fs.copy(path.join(PLATFORMS_DIR, platform), __dirname, {
  filter: (src) => !IGNORED_ARTIFACTS.includes(path.basename(src))
});

async function installPlatformArtifacts() {
  try {
    await fs.ensureFile(ENTRYPOINT);
    console.log('binary already installed');
    return;
  } catch {
    // install binaries if they do not exist
  }

  switch (process.platform) {
    case 'darwin':
      await copyArtifacts('macos');
      break;
    case 'linux':
      await copyArtifacts('linux');
      break;
    default:
      console.error('This platform (' + process.platform + ') is not supported yet.');
      process.exit(-1);
  }

  // TODO: enable when confident
  // await fs.remove(PLATFORMS_DIR);
  
  const { stdout } = await execa('node', [ENTRYPOINT], {
    env: {
      ESY_RELEASE_REWRITE_PREFIX: 'true'
    }
  });
  console.log(stdout);
}

installPlatformArtifacts();