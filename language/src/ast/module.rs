use super::import::Import;
use crate::ast::declaration::DeclarationNode;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub struct Module<D> {
    pub imports: Vec<Import>,
    pub declarations: Vec<D>,
}

impl<D> Module<D> {
    pub fn new(imports: Vec<Import>, declarations: Vec<D>) -> Self {
        Self {
            imports,
            declarations,
        }
    }

    pub fn map<D2>(&self, fd: &impl Fn(&D) -> D2) -> Module<D2> {
        Module {
            imports: self.imports.clone(),
            declarations: self.declarations.iter().map(fd).collect(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.imports.is_empty() || self.declarations.is_empty()
    }
}

pub type ModuleNodeValue<C> = Module<DeclarationNode<C>>;

#[derive(Debug, PartialEq)]
pub struct ModuleNode<C>(pub ModuleNodeValue<C>, pub C);

impl<C> ModuleNode<C> {
    pub fn map<C2>(
        &self,
        f: impl Fn(&ModuleNodeValue<C>, &C) -> (ModuleNodeValue<C2>, C2),
    ) -> ModuleNode<C2> {
        let (value, ctx) = f(&self.0, &self.1);

        ModuleNode(value, ctx)
    }
}

impl ModuleNode<()> {
    pub fn raw(x: ModuleNodeValue<()>) -> Self {
        Self(x, ())
    }
}
