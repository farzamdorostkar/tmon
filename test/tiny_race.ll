; ModuleID = 'tiny_race.c'
source_filename = "tiny_race.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Global = dso_local local_unnamed_addr global i32 0, align 4, !dbg !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local ptr @Thread1(ptr noundef readnone returned %0) #0 !dbg !16 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !20, metadata !DIExpression()), !dbg !21
  store i32 42, ptr @Global, align 4, !dbg !22, !tbaa !23
  ret ptr %0, !dbg !27
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 !dbg !28 {
  %1 = alloca i64, align 8, !DIAssignID !36
  call void @llvm.dbg.assign(metadata i1 undef, metadata !32, metadata !DIExpression(), metadata !36, metadata ptr %1, metadata !DIExpression()), !dbg !37
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %1) #7, !dbg !38
  %2 = call i32 @pthread_create(ptr noundef nonnull %1, ptr noundef null, ptr noundef nonnull @Thread1, ptr noundef null) #7, !dbg !39
  store i32 43, ptr @Global, align 4, !dbg !40, !tbaa !23
  %3 = load i64, ptr %1, align 8, !dbg !41, !tbaa !42
  %4 = call i32 @pthread_join(i64 noundef %3, ptr noundef null) #7, !dbg !44
  %5 = load i32, ptr @Global, align 4, !dbg !45, !tbaa !23
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %1) #7, !dbg !46
  ret i32 %5, !dbg !47
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nounwind
declare !dbg !48 i32 @pthread_create(ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

declare !dbg !69 i32 @pthread_join(i64 noundef, ptr noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.assign(metadata, metadata, metadata, metadata, metadata, metadata) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #6

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #7 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!8, !9, !10, !11, !12, !13, !14}
!llvm.ident = !{!15}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "Global", scope: !2, file: !3, line: 3, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 17.0.6 (/home/farzam/code/llvm-project-17.0.6.src/clang 4ed116c3265bbfd6d606bf4a33f8862b18de8956)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !6, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "tiny_race.c", directory: "/home/farzam/code/tmon_test/test", checksumkind: CSK_MD5, checksum: "8d6bccf7133cfb20029c2cdca1b3bf20")
!4 = !{!5}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!6 = !{!0}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{i32 7, !"Dwarf Version", i32 5}
!9 = !{i32 2, !"Debug Info Version", i32 3}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{i32 8, !"PIC Level", i32 2}
!12 = !{i32 7, !"PIE Level", i32 2}
!13 = !{i32 7, !"uwtable", i32 2}
!14 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!15 = !{!"clang version 17.0.6 (/home/farzam/code/llvm-project-17.0.6.src/clang 4ed116c3265bbfd6d606bf4a33f8862b18de8956)"}
!16 = distinct !DISubprogram(name: "Thread1", scope: !3, file: !3, line: 5, type: !17, scopeLine: 5, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{!5, !5}
!19 = !{!20}
!20 = !DILocalVariable(name: "x", arg: 1, scope: !16, file: !3, line: 5, type: !5)
!21 = !DILocation(line: 0, scope: !16)
!22 = !DILocation(line: 6, column: 10, scope: !16)
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
!27 = !DILocation(line: 7, column: 3, scope: !16)
!28 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 10, type: !29, scopeLine: 10, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !31)
!29 = !DISubroutineType(types: !30)
!30 = !{!7}
!31 = !{!32}
!32 = !DILocalVariable(name: "t", scope: !28, file: !3, line: 11, type: !33)
!33 = !DIDerivedType(tag: DW_TAG_typedef, name: "pthread_t", file: !34, line: 27, baseType: !35)
!34 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h", directory: "", checksumkind: CSK_MD5, checksum: "735e3bf264ff9d8f5d95898b1692fbdb")
!35 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!36 = distinct !DIAssignID()
!37 = !DILocation(line: 0, scope: !28)
!38 = !DILocation(line: 11, column: 3, scope: !28)
!39 = !DILocation(line: 12, column: 3, scope: !28)
!40 = !DILocation(line: 13, column: 10, scope: !28)
!41 = !DILocation(line: 14, column: 16, scope: !28)
!42 = !{!43, !43, i64 0}
!43 = !{!"long", !25, i64 0}
!44 = !DILocation(line: 14, column: 3, scope: !28)
!45 = !DILocation(line: 15, column: 10, scope: !28)
!46 = !DILocation(line: 16, column: 1, scope: !28)
!47 = !DILocation(line: 15, column: 3, scope: !28)
!48 = !DISubprogram(name: "pthread_create", scope: !49, file: !49, line: 202, type: !50, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!49 = !DIFile(filename: "/usr/include/pthread.h", directory: "", checksumkind: CSK_MD5, checksum: "5205981c6f80cc3dc1e81231df63d8ef")
!50 = !DISubroutineType(types: !51)
!51 = !{!7, !52, !54, !67, !68}
!52 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !53)
!53 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!54 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !55)
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !56, size: 64)
!56 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !57)
!57 = !DIDerivedType(tag: DW_TAG_typedef, name: "pthread_attr_t", file: !34, line: 62, baseType: !58)
!58 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "pthread_attr_t", file: !34, line: 56, size: 448, elements: !59)
!59 = !{!60, !65}
!60 = !DIDerivedType(tag: DW_TAG_member, name: "__size", scope: !58, file: !34, line: 58, baseType: !61, size: 448)
!61 = !DICompositeType(tag: DW_TAG_array_type, baseType: !62, size: 448, elements: !63)
!62 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!63 = !{!64}
!64 = !DISubrange(count: 56)
!65 = !DIDerivedType(tag: DW_TAG_member, name: "__align", scope: !58, file: !34, line: 59, baseType: !66, size: 64)
!66 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!67 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!68 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !5)
!69 = !DISubprogram(name: "pthread_join", scope: !49, file: !49, line: 219, type: !70, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!70 = !DISubroutineType(types: !71)
!71 = !{!7, !33, !72}
!72 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
