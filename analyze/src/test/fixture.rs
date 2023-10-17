use crate::{
    context::{BindingMap, FileContext, FragmentMap, ScopeContext, StrongContext},
    fragment::Fragment,
    infer::strong::StrongRef,
};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[allow(dead_code)]
pub fn file_ctx() -> RefCell<FileContext> {
    RefCell::new(FileContext::new())
}

#[allow(dead_code)]
pub fn file_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> FileContext {
    let ctx = file_ctx();
    ctx.borrow_mut().fragments.0.extend(xs);
    ctx.into_inner()
}

#[allow(dead_code)]
pub fn scope_ctx<'a>(file_ctx: &'a RefCell<FileContext>) -> ScopeContext<'a> {
    ScopeContext::new(file_ctx)
}

#[allow(dead_code)]
pub fn strong_ctx_from(
    fragments: Vec<(usize, (Vec<usize>, Fragment))>,
    refs: Vec<(usize, StrongRef)>,
    bindings: Vec<((Vec<usize>, String), BTreeSet<usize>)>,
) -> StrongContext {
    let mut ctx = StrongContext::new(
        FragmentMap(BTreeMap::from_iter(fragments)),
        BindingMap(HashMap::from_iter(bindings)),
    );
    ctx.refs.extend(refs);
    ctx
}
