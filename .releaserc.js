module.exports = {
  tagFormat: 'v${version}-dev',
  prepare: [
    '@semantic-release/changelog',
    '@semantic-release/npm',
    '@semantic-release/git'
  ],
  publish: [
    '@semantic-release/npm'
  ],
  verifyConditions: [
    '@semantic-release/npm',
    '@semantic-release/git'
  ],
  monorepo: {
    analyzeCommits: [
      '@semantic-release/commit-analyzer'
    ],
    generateNotes: [
      '@semantic-release/release-notes-generator'
    ]
  }
};
