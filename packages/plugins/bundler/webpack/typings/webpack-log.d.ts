declare module 'webpack-log' {
  type Config = {
    name?: string;
    timestamp?: boolean;
    unique?: boolean;
    level?: 'trace' | 'debug' | 'info' | 'warn' | 'error' | 'silent';
  };

  type Logger = (...args: any[]) => void;

  const getLogger: (
    config: Config
  ) => {
    trace: Logger;
    debug: Logger;
    info: Logger;
    warn: Logger;
    error: Logger;
    silent: Logger;
  };

  export default getLogger;
}
