module.exports = {
  parser: '@typescript-eslint/parser',
  plugins: [
    '@typescript-eslint',
    'prettier',
    'sonarjs',
    'optimize-regex',
    'filenames',
    'simple-import-sort'
  ],
  extends: [
    'plugin:@typescript-eslint/recommended',
    'prettier',
    'plugin:eslint-comments/recommended',
    'plugin:sonarjs/recommended',
    'plugin:promise/recommended'
  ],
  rules: {
    'prettier/prettier': 'error',
    'optimize-regex/optimize-regex': 'error',
    'simple-import-sort/sort': 'error',
    'eslint-comments/disable-enable-pair': ['error', { allowWholeFile: true }],

    // disabled
    '@typescript-eslint/no-explicit-any': 'off',
    '@typescript-eslint/explicit-function-return-type': 'off',
    '@typescript-eslint/no-unused-vars': 'off',
    'filenames/match-regex': 'off',
    'filenames/match-exported': 'off'
  }
};
