use super::{code::ToCode, Display, ErrorCode, ErrorDisplay};
use kore::pretty::Pretty;
use std::{io, path::PathBuf};

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum EnvironmentError {
    InvalidWriteTarget(PathBuf, io::ErrorKind),
    CleanupFailed(PathBuf, io::ErrorKind),
    MaxFilesWatched(PathBuf),
    WatchFailed(PathBuf, String),
}

impl ToCode for EnvironmentError {
    fn to_code(&self) -> ErrorCode {
        match self {
            Self::InvalidWriteTarget(.., io::ErrorKind::NotFound) => {
                ErrorCode::INVALID_WRITE_TARGET_NOT_FOUND
            }
            Self::InvalidWriteTarget(.., io::ErrorKind::PermissionDenied) => {
                ErrorCode::INVALID_WRITE_TARGET_PERMISSION_DENIED
            }
            Self::InvalidWriteTarget(..) => ErrorCode::INVALID_WRITE_TARGET,
            Self::CleanupFailed(..) => ErrorCode::CLEANUP_FAILED,
            Self::MaxFilesWatched(..) => ErrorCode::MAX_FILES_WATCHED,
            Self::WatchFailed(..) => ErrorCode::WATCH_FAILED,
        }
    }
}

impl<'a> Display<'a> for EnvironmentError {
    type Context = ();

    fn display(&'a self, (): Self::Context) -> ErrorDisplay<'a> {
        let bind = |title, description| ErrorDisplay::simple(self.to_code(), title, description);

        match self {
            Self::InvalidWriteTarget(path, error) => bind(
                "Invalid Write Target",
                format!(
                    "Attempted write to {} failed with error {error}.",
                    path.pretty()
                ),
            ),

            Self::CleanupFailed(path, error) => bind(
                "Cleanup Failed",
                format!(
                    "Attempted to delete {} but failed with error {error}.",
                    path.pretty()
                ),
            ),

            Self::MaxFilesWatched(path) => bind(
                "Max Files Watched",
                format!(
                    "Cannot watch {} because the file notifier limit has been reached.",
                    path.pretty()
                ),
            ),

            Self::WatchFailed(path, error) => bind(
                "Watch Failed",
                format!(
                    "Attempted to watch {} but failed with error {error}.",
                    path.pretty()
                ),
            ),
        }
    }
}
