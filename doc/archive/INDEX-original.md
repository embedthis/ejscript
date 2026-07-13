# Ejscript Documentation Index

Quick reference index for all project documentation.

## Primary Documents (Root Level)

| Document | Purpose | When to Read |
|----------|---------|--------------|
| [CLAUDE.md](../CLAUDE.md) | AI assistant guidance | First stop for Claude Code |
| [README.md](../README.md) | Project overview | User-facing introduction |
| [QUICK_START.md](../QUICK_START.md) | Quick start guide | Getting started quickly |
| [IMPLEMENTATION.md](../IMPLEMENTATION.md) | Implementation details | Understanding how it works |
| [PROJECT_SUMMARY.md](../PROJECT_SUMMARY.md) | Complete feature list | Full feature overview |
| [STATUS.md](../STATUS.md) | Current status | Quick status check |

## AI/ Documentation Structure

| Document | Lines | Purpose |
|----------|-------|---------|
| [README.md](README.md) | 250 | Documentation overview |
| [context/CURRENT.md](context/CURRENT.md) | 225 | Current project state |
| [designs/DESIGN.md](designs/DESIGN.md) | 450 | Architecture & design |
| [plans/PLAN.md](plans/PLAN.md) | 550 | Roadmap & planning |
| [procedures/PROCEDURES.md](procedures/PROCEDURES.md) | 550 | Development workflows |
| [logs/CHANGELOG.md](logs/CHANGELOG.md) | 430 | Complete change history |
| [references/REFERENCES.md](references/REFERENCES.md) | 360 | External resources |

**Total**: ~2,165 lines of structured documentation

## Reading Paths

### For New Developers
```
1. CLAUDE.md → Quick overview and build commands
2. designs/DESIGN.md → Architecture understanding  
3. procedures/PROCEDURES.md → How to develop
4. context/CURRENT.md → Current state
```

### For AI Assistants
```
1. CLAUDE.md → Primary guidance
2. context/CURRENT.md → Current context
3. designs/DESIGN.md → Architecture details
4. procedures/PROCEDURES.md → Development procedures
```

### For Adding Features
```
1. designs/DESIGN.md → Understand architecture
2. plans/PLAN.md → Check roadmap alignment
3. procedures/PROCEDURES.md → Follow workflow
4. context/CURRENT.md → Check current state
```

### For Bug Fixes
```
1. context/CURRENT.md → Check known issues
2. procedures/PROCEDURES.md → Testing procedures
3. designs/DESIGN.md → Understand component design
```

### For Releases
```
1. procedures/PROCEDURES.md → Release procedures
2. logs/CHANGELOG.md → Document changes
3. plans/PLAN.md → Update roadmap if needed
```

## Documentation by Topic

### Architecture & Design
- [designs/DESIGN.md](designs/DESIGN.md) - Complete architecture
- Component designs
- Design patterns
- Performance considerations

### Development
- [procedures/PROCEDURES.md](procedures/PROCEDURES.md) - All procedures
- Setup, building, testing
- Code review process
- Troubleshooting

### Planning & Roadmap
- [plans/PLAN.md](plans/PLAN.md) - Complete plan
- Completed phases
- Future enhancements
- Maintenance plan

### Current State
- [context/CURRENT.md](context/CURRENT.md) - Current context
- Recent activities
- Active development
- Status dashboard

### History
- [logs/CHANGELOG.md](logs/CHANGELOG.md) - All changes
- Version history
- Implementation timeline

### External Resources
- [references/REFERENCES.md](references/REFERENCES.md) - All references
- Official documentation links
- Tools and utilities
- Learning resources

## Quick Commands

```bash
# View any document
cat AI/designs/DESIGN.md
cat AI/plans/PLAN.md
cat AI/procedures/PROCEDURES.md
cat AI/logs/CHANGELOG.md

# Search documentation
grep -r "search term" AI/

# List all documents
find .agent -name "*.md" -type f

# Count documentation lines
wc -l AI/**/*.md
```

## Maintenance Schedule

- **Daily**: Update CURRENT.md for significant changes
- **Per Change**: Update CHANGELOG.md
- **Weekly**: Review context/CURRENT.md
- **Monthly**: Review all documentation for accuracy
- **Quarterly**: Archive outdated documents
- **Per Release**: Update all relevant documents

## Document Relationships

```
CLAUDE.md (root)
    ├─ Guides to → context/CURRENT.md
    ├─ References → designs/DESIGN.md
    └─ Links to → procedures/PROCEDURES.md

context/CURRENT.md
    ├─ Summary of → plans/PLAN.md
    ├─ References → designs/DESIGN.md
    └─ Points to → procedures/PROCEDURES.md

designs/DESIGN.md
    ├─ Referenced by → plans/PLAN.md
    └─ Informs → procedures/PROCEDURES.md

procedures/PROCEDURES.md
    ├─ Uses → designs/DESIGN.md
    └─ Updates → logs/CHANGELOG.md

plans/PLAN.md
    ├─ Based on → designs/DESIGN.md
    └─ Tracked in → logs/CHANGELOG.md
```

## Archive Structure

When archiving outdated documents:
```
AI/archive/
    ├─ YYYY-MM-DD/
    │   ├─ designs/
    │   ├─ plans/
    │   ├─ procedures/
    │   └─ context/
```

## Getting Help

1. **Start with**: [CLAUDE.md](../CLAUDE.md)
2. **For architecture**: [designs/DESIGN.md](designs/DESIGN.md)
3. **For procedures**: [procedures/PROCEDURES.md](procedures/PROCEDURES.md)
4. **For status**: [context/CURRENT.md](context/CURRENT.md)
5. **For history**: [logs/CHANGELOG.md](logs/CHANGELOG.md)

---

**Last Updated**: 2025-10-17
**Documentation Version**: 1.0
