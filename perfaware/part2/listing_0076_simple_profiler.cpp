/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 76
   ======================================================================== */

#include "listing_0074_platform_metrics.cpp"

struct profile_anchor
{
    u64 TSCElapsed;
    u64 HitCount;
    char const *Label;
};

struct profiler
{
    profile_anchor Anchors[4096];
    
    u64 StartTSC;
    u64 EndTSC;
};
static profiler GlobalProfiler;

template <std::size_t N>
struct fixed_string {
    char chars[N] = {};
    explicit(false) constexpr fixed_string(const char (&str)[N])
    {
        // std::copy_n(std::begin(str), N, std::begin(chars));
        for (std::size_t i = 0; i < N; ++i) {
            chars[i] = str[i];
        }
    }
};

template <std::size_t N>
fixed_string(const char (&str)[N]) -> fixed_string<N>;

template <u32 AnchorIndex, fixed_string Label>
struct label_registrator {
    label_registrator() { GlobalProfiler.Anchors[AnchorIndex].Label = Label.chars; }
};

template <u32 AnchorIndex, fixed_string Label>
label_registrator<AnchorIndex, Label> register_label{};

template <u32 AnchorIndex, fixed_string Label, auto& = register_label<AnchorIndex, Label>>
struct profile_block
{
    profile_block()
    {
        StartTSC = ReadCPUTimer();
    }
    
    ~profile_block()
    {
        u64 Elapsed = ReadCPUTimer() - StartTSC;
        
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        Anchor->TSCElapsed += Elapsed;
        ++Anchor->HitCount;
    }

    u64 StartTSC;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) profile_block<__COUNTER__ + 1, Name> NameConcat(Block, __LINE__){};
#define TimeFunction TimeBlock(__func__)

static void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
{
    u64 Elapsed = Anchor->TSCElapsed;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s[%llu]: %llu (%.2f%%)\n", Anchor->Label, Anchor->HitCount, Elapsed, Percent);
}

static void BeginProfile(void)
{
    GlobalProfiler.StartTSC = ReadCPUTimer();
}

static void EndAndPrintProfile()
{
    GlobalProfiler.EndTSC = ReadCPUTimer();
    u64 CPUFreq = EstimateCPUTimerFreq();
    
    u64 TotalCPUElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;
    
    if(CPUFreq)
    {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
    }
    
    for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GlobalProfiler.Anchors); ++AnchorIndex)
    {
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        if(Anchor->TSCElapsed)
        {
            PrintTimeElapsed(TotalCPUElapsed, Anchor);
        }
    }
}
