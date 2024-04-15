use crate::walk::{Visit, Walk, WalkEach};
use std::fmt::Debug;

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ImportSource {
    Root,
    Local,
    Named(String),
    Scoped { scope: String, name: String },
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Import {
    pub source: ImportSource,
    pub path: Vec<String>,
    pub alias: Option<String>,
}

impl Import {
    pub fn new(source: ImportSource, path: Vec<String>, alias: Option<String>) -> Self {
        Self {
            source,
            path,
            alias,
        }
    }
}

impl<Visitor, Context> Walk<Visitor> for (Import, Context)
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Import;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            Import {
                source,
                path,
                alias,
            },
            ctx,
        ) = self;

        v.import(
            Import {
                source,
                path,
                alias,
            },
            ctx,
        )
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Module<Import, Declaration> {
    pub imports: Vec<Import>,
    pub declarations: Vec<Declaration>,
}

impl<Import, Declaration> Module<Import, Declaration> {
    pub fn new(imports: Vec<Import>, declarations: Vec<Declaration>) -> Self {
        Self {
            imports,
            declarations,
        }
    }
}

impl<Visitor, Context, Import, Declaration> Walk<Visitor> for (Module<Import, Declaration>, Context)
where
    Visitor: Visit<Context = Context>,
    Import: Walk<Visitor, Output = Visitor::Import>,
    Declaration: Walk<Visitor, Output = Visitor::Declaration>,
{
    type Output = Visitor::Module;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (
            Module {
                imports,
                declarations,
            },
            ctx,
        ) = self;
        let ((imports, declarations), v) = (imports, declarations).walk_each(v);

        v.module(
            Module {
                imports,
                declarations,
            },
            ctx,
        )
    }
}
