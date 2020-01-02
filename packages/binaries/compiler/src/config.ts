import nodePath from 'path';

export const path = nodePath.join(__dirname, '..', '..', 'bin', 'knotc.exe');

export const KNOT_BINARY = process.env.KNOT_BINARY || path;
