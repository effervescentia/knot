import nodePath from 'path';

export const binary = nodePath.join(__dirname, '..', '..', 'bin', 'knotc.exe');

export const KNOTC_BINARY = process.env.KNOTC_BINARY || binary;
