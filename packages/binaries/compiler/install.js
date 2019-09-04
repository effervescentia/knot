#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const execa = require('execa');

const ENTRYPOINT = 'esyInstallRelease.js';
const ARTIFACTS = ['bin', '_export', ENTRYPOINT]
const PLATFORMS_DIR = path.join(__dirname, 'platforms')

const copyArtifacts = platform => fs.copy(path.join(PLATFORMS_DIR, platform, '{' + ARTIFACTS.join(',') + '}'), __dirname);

async function installPlatformArtifacts() {
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

  await fs.remove(PLATFORMS_DIR);
  
  const { stdout } = await execa('node', [path.join(__dirname, ENTRYPOINT)]);
  console.log(stdout);
}

installPlatformArtifacts();