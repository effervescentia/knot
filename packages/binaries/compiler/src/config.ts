import path from 'path';

export const binary = path.join(__dirname, '..', '..', 'bin', 'knotc.exe');

export const KNOTC_BINARY = process.env.KNOTC_BINARY || binary;
