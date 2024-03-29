mod format;
mod javascript;
mod transform;

pub use javascript::JavaScript;
use kore::{str, Generator};
use lang::ast;
use std::path::{Path, PathBuf};

// TODO: move these to a common library to be re-used across generators

#[derive(PartialEq)]
pub enum Mode {
    Dev,
    Prod,
}

#[derive(Clone, Copy)]
pub enum Module {
    /// ECMAScript modules
    ESM,

    /// CommonJS modules
    CJS,
}

impl Default for Module {
    fn default() -> Self {
        Self::ESM
    }
}

pub struct Options {
    pub mode: Mode,
    pub module: Module,
}

#[derive(Clone, Copy)]
pub struct JavaScriptGenerator(Module);

impl JavaScriptGenerator {
    pub const fn new(module: Module) -> Self {
        Self(module)
    }
}

impl Generator for JavaScriptGenerator {
    type Input = ast::shape::Module;
    type Output = JavaScript;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output) {
        let mut path_to_root = path
            .parent()
            .map(|x| x.iter().map(|_| "..").collect::<Vec<_>>().join("/"))
            .unwrap_or_default();
        if path_to_root.is_empty() {
            path_to_root = str!(".");
        }

        (
            path.with_extension("js"),
            JavaScript::from_module(
                &path_to_root,
                &input,
                &Options {
                    mode: Mode::Prod,
                    module: self.0,
                },
            ),
        )
    }
}
