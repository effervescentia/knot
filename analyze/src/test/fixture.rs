// #![allow(dead_code)]

// use crate::{
//     context::{BindingMap, FileContext, FragmentMap, ScopeContext, StrongContext},
//     fragment::Fragment,
//     infer::strong::StrongRef,
// };
// pub use parse::test::fixture::{a, n};
// use std::{
//     cell::RefCell,
//     collections::{BTreeMap, BTreeSet, HashMap},
// };

// pub const fn file_ctx() -> RefCell<FileContext> {
//     RefCell::new(FileContext::new())
// }

// pub fn file_ctx_from(xs: Vec<(usize, (Vec<usize>, Fragment))>) -> FileContext {
//     let ctx = file_ctx();
//     ctx.borrow_mut().fragments.0.extend(xs);
//     ctx.into_inner()
// }

// pub fn scope_ctx(file_ctx: &RefCell<FileContext>) -> ScopeContext {
//     ScopeContext::new(file_ctx)
// }

use crate::{
    context::{BindingMap, ProgramContext, StrongContext},
    infer::strong::StrongRef,
};
use lang::ast::{
    explode::{self, ScopeId},
    walk::NodeId,
};
use std::collections::{BTreeMap, BTreeSet, HashMap};

pub fn strong_ctx_from(
    fragments: Vec<(NodeId, (ScopeId, explode::Fragment))>,
    refs: Vec<(NodeId, StrongRef)>,
    bindings: Vec<((ScopeId, String), BTreeSet<NodeId>)>,
) -> StrongContext {
    let mut ctx = StrongContext::new(ProgramContext::new(
        explode::FragmentMap(BTreeMap::from_iter(fragments)),
        BindingMap(HashMap::from_iter(bindings)),
    ));
    ctx.refs.extend(refs);
    ctx
}
