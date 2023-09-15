use super::{fragment::Fragment, StrongRef, WeakRef};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[derive(Debug, PartialEq)]
pub struct FileContext {
    next_scope_id: usize,
    next_fragment_id: usize,

    pub fragments: BTreeMap<usize, (Vec<usize>, Fragment)>,
}

impl FileContext {
    pub fn new() -> Self {
        Self {
            next_scope_id: 0,
            next_fragment_id: 0,
            fragments: BTreeMap::new(),
        }
    }

    pub fn scope_id(&mut self) -> usize {
        let id = self.next_scope_id;
        self.next_scope_id += 1;
        id
    }

    pub fn fragment_id(&mut self) -> usize {
        let id = self.next_fragment_id;
        self.next_fragment_id += 1;
        id
    }

    pub fn add_fragment(&mut self, scope: Vec<usize>, fragment: Fragment) -> usize {
        let id = self.fragment_id();
        self.fragments.insert(id, (scope, fragment));
        id
    }
}

pub struct ScopeContext<'a> {
    id: usize,

    parent: Option<&'a ScopeContext<'a>>,

    pub file: &'a RefCell<FileContext>,
}

impl<'a> ScopeContext<'a> {
    pub fn new(file: &'a RefCell<FileContext>) -> Self {
        let id = file.borrow_mut().scope_id();

        Self {
            id,
            parent: None,
            file,
        }
    }

    pub fn child(&'a self) -> Self {
        let id = self.file.borrow_mut().scope_id();

        Self {
            id,
            parent: Some(self),
            file: self.file,
        }
    }

    pub fn path(&self) -> Vec<usize> {
        match self.parent {
            Some(x) => vec![x.path(), vec![self.id]].concat(),
            None => vec![self.id],
        }
    }

    pub fn add_fragment(&mut self, x: Fragment) -> NodeContext {
        NodeContext::new(
            self.file.borrow_mut().add_fragment(self.path(), x),
            self.path(),
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct NodeContext {
    scope: Vec<usize>,
    id: usize,
}

impl NodeContext {
    pub fn new(id: usize, scope: Vec<usize>) -> Self {
        Self { id, scope }
    }

    pub fn scope<'a>(&'a self) -> &'a Vec<usize> {
        &self.scope
    }

    pub fn id<'a>(&'a self) -> &'a usize {
        &self.id
    }
}

#[derive(Debug, PartialEq)]
pub struct AnalyzeContext<'a> {
    pub file: &'a RefCell<FileContext>,

    pub bindings: HashMap<(Vec<usize>, String), BTreeSet<usize>>,

    pub weak_refs: HashMap<usize, WeakRef>,

    pub strong_refs: HashMap<usize, StrongRef>,
}

impl<'a> AnalyzeContext<'a> {
    pub fn new(file: &'a RefCell<FileContext>) -> Self {
        Self {
            file,
            bindings: HashMap::new(),
            weak_refs: HashMap::new(),
            strong_refs: HashMap::new(),
        }
    }
}
