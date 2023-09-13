use super::{fragment::Fragment, WeakRef};
use std::{borrow::BorrowMut, cell::RefCell, collections::HashMap, fs::File};

pub struct FileContext {
    next_scope_id: usize,
    next_fragment_id: usize,

    pub fragments: HashMap<usize, Fragment>,

    pub weak_refs: HashMap<usize, WeakRef>,
}

impl FileContext {
    pub fn new() -> Self {
        Self {
            next_scope_id: 0,
            next_fragment_id: 0,
            fragments: HashMap::new(),
            weak_refs: HashMap::new(),
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

    pub fn add_fragment(&mut self, x: Fragment) -> usize {
        let id = self.fragment_id();
        self.fragments.insert(id, x);
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
        Self {
            id: 0,
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

    pub fn add_fragment(&mut self, x: Fragment) -> usize {
        self.file.borrow_mut().add_fragment(x)
    }
}
