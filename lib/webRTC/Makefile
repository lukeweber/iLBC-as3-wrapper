CC = gcc
ARCH=$(PROC)

ILBC_PATH=webrtc/src/modules/audio_coding/codecs/ilbc/
SPL_PATH=webrtc/src/common_audio/signal_processing/

C_INCLUDES := \
    -Iwebrtc/src/ \
    -I$(ILBC_PATH)interface/ \
    -I$(ILBC_PATH) \
    -I$(SPL_PATH)include/

CFLAGS+= -Wall -O3
CC_SWITCHES = -Wall -O3 $(C_INCLUDES)

PNAME = iLBC_webrtc

ILBC_SOURCES :=\
    abs_quant.c \
    abs_quant_loop.c \
    augmented_cb_corr.c \
    bw_expand.c \
    cb_construct.c \
    cb_mem_energy.c \
    cb_mem_energy_augmentation.c \
    cb_mem_energy_calc.c \
    cb_search.c \
    cb_search_core.c \
    cb_update_best_index.c \
    chebyshev.c \
    comp_corr.c \
    constants.c \
    create_augmented_vec.c \
    decode.c \
    decode_residual.c \
    decoder_interpolate_lsf.c \
    do_plc.c \
    encode.c \
    energy_inverse.c \
    enh_upsample.c \
    enhancer.c \
    enhancer_interface.c \
    filtered_cb_vecs.c \
    frame_classify.c \
    gain_dequant.c \
    gain_quant.c \
    get_cd_vec.c \
    get_lsp_poly.c \
    get_sync_seq.c \
    hp_input.c \
    hp_output.c \
    ilbc.c \
    index_conv_dec.c \
    index_conv_enc.c \
    init_decode.c \
    init_encode.c \
    interpolate.c \
    interpolate_samples.c \
    lpc_encode.c \
    lsf_check.c \
    lsf_interpolate_to_poly_dec.c \
    lsf_interpolate_to_poly_enc.c \
    lsf_to_lsp.c \
    lsf_to_poly.c \
    lsp_to_lsf.c \
    my_corr.c \
    nearest_neighbor.c \
    pack_bits.c \
    poly_to_lsf.c \
    poly_to_lsp.c \
    refiner.c \
    simple_interpolate_lsf.c \
    simple_lpc_analysis.c \
    simple_lsf_dequant.c \
    simple_lsf_quant.c \
    smooth.c \
    smooth_out_data.c \
    sort_sq.c \
    split_vq.c \
    state_construct.c \
    state_search.c \
    swap_bytes.c \
    unpack_bits.c \
    vq3.c \
    vq4.c \
    window32_w32.c \
    xcorr_coef.c

SPL_SOURCES :=\
    auto_corr_to_refl_coef.c \
    auto_correlation.c \
    complex_fft.c \
    copy_set_operations.c \
    division_operations.c \
    dot_product_with_scale.c \
    energy.c \
    filter_ar.c \
    filter_ma_fast_q12.c \
    get_hanning_window.c \
    get_scaling_square.c \
    ilbc_specific_functions.c \
    levinson_durbin.c \
    lpc_to_refl_coef.c \
    min_max_operations.c \
    randomization_functions.c \
    refl_coef_to_lpc.c \
    resample.c \
    resample_48khz.c \
    resample_by_2.c \
    resample_by_2_internal.c \
    resample_fractional.c \
    spl_sqrt.c \
    spl_version.c \
    splitting_filter.c \
    sqrt_of_one_minus_x_squared.c \
    vector_scaling_operations.c \
    filter_ar_fast_q12.c \
    cross_correlation.c \
    downsample_fast.c \
    complex_bit_reverse.c \
    spl_sqrt_floor.c

MAIN_SOURCE = iLBC_as3_wrapper.c

RES_SPL_SOURCES = $(addprefix $(SPL_PATH), $(SPL_SOURCES))
RES_ILBC_SOURCES = $(addprefix $(ILBC_PATH), $(ILBC_SOURCES))

ILBC = $(RES_ILBC_SOURCES:.c=.o)
SPL = $(RES_SPL_SOURCES:.c=.o)
MAIN = $(MAIN_SOURCE:.c=.o)

%.o: %.c
	$(CC) $(CC_SWITCHES) -c $< -o $@

extract:
	svn co http://webrtc.googlecode.com/svn/trunk webrtc

compileswc: $(ILBC) $(SPL) $(MAIN)
	$(CC) $(CFLAGS) -swc -o $(PNAME).swc $(ILBC) $(SPL) $(MAIN)
	mv $(PNAME).swc ../../bin/

all: compileswc clean

clean:
	rm $(IBLC) $(SPL) $(MAIN)
	cd $(ILBC_PATH)
	rm -f core *.bak *.a *.exe *.exe.bc *.l.bc *.achacks.* *.stackdump *.swc
	cd -
	cd $(SPL_PATH)
	rm -f core *.bak *.a *.exe *.exe.bc *.l.bc *.achacks.* *.stackdump *.swc
	cd -
