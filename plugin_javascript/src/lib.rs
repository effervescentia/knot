mod format;
mod javascript;
mod transform;

pub use javascript::JavaScript;
use kore::Generator;
use lang::ast::ProgramShape;
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
    pub fn new(module: Module) -> Self {
        Self(module)
    }
}

impl Generator for JavaScriptGenerator {
    type Input = ProgramShape;
    type Output = JavaScript;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output) {
        (
            path.with_extension("js"),
            JavaScript::from_module(
                &input.0,
                &Options {
                    mode: Mode::Prod,
                    module: self.0,
                },
            ),
        )
    }
}
