#ifndef V
#define V(key, value)
#endif
V(enabled, "启用(重启生效)")
V(lang_name, "chinese")

V(heart_config, "心率设置")
V(heart_devices, "心率设备")
V(heart_senders, "心率数据源")
V(heart_osc_senders, "OSC心率源")

V(hyperate, "HypeRate")

// TODO: translate V(pulsoid, "Pulsoid")
V(hyperate_reset, "重置")
V(hyperate_save_and_connect, "保存并连接")
V(hyperate_input_hint, "https://app.hyperate.io/<color=\"green\">xxx</color><-输入这个")

V(no_message_from_server, "")

V(hyperate_con_start, "开始连接")
V(hyperate_network_error, "网络错误: ")
V(hyperate_retry, "重试")
V(hyperate_connected, "已连接")
V(hyperate_no_id, "未填写 hyperate ID")
V(hyperate_refused, "服务器拒绝服务")
V(hyperate_you_have_message, "收到一条消息")

V(pulsoid_no_token, "无可用token")

V(unsupported_feature_udpatre_mod, "UI不支持,可能需要升级MOD")

V(heart_osc_port, "OSC端口号: %d")

V(scan_start, "开始扫描")
V(scan_stop, "停止扫描")
V(turn_location_on, "请打开精准位置服务")

V(scaning, "正在扫描")
V(auto_scaning, "正在自动扫描")
V(no_scan, "扫描已停止")

V(ble_none_selected, ">> 无")
V(ble_none_not_selected, "无")

V(ble_permission_unknown, "权限清单获取错误")
V(ble_permission_needed, "游戏蓝牙权限缺失")
V(ble_permission_paired_only, "仅配对设备，可能需要更多权限")

V(pairing, "正在配对，点击停止")
V(not_pairing, "配对停止，点击开始")
V(hiding_mac_addr, "MAC地址已隐藏，点击显示")
V(showing_mac_addr, "MAC地址已显示，点击隐藏")
V(unknown_left_quote, "未知(")
V(server_skip, "已跳过 ")
V(unknown_addr_server_fmt, "%s 未知地址。")
V(heart_rate_lan_protocol_ver, "Heart Rate Lan 协议版本：")
V(mod_version, "模组版本 " VERSION)
V(for_game, "适配游戏版本" GAME_VERSION)
V(no_beatleader, "Beatleader未找到或不支持，录制已禁用")
V(waiting, "等待中...")

// TODO: translate V(config_adjust_speed, "Config Adjust Speed")
V(text_color, "文字颜色")
V(flash_text_color, "闪烁颜色")
V(replay_flash_text_color, "录像闪烁颜色")
V(flash_duration_when_text_come, "数据抵达时的闪烁时长")
V(flash_test, "闪烁测试")
// TODO: translate V(line_space, "Line Space")
// TODO: translate V(reset_default_line_space, "Reset Default Line Space")
V(reset_to_default, "恢复默认")

V(data_source, "数据源(重启生效)")

//BUG: can't translate text inside dropdown
//V(data_source_random, "随机")
//V(data_source_lan, "局域网")
//V(data_source_bluetooth, "蓝牙")
// TODO: translate V(data_source_osc, "OSC")
// TODO: translate V(data_source_hyperate, "HypeRate")
// TODO: translate V(data_source_pulsoid, "Pulsoid")
V(enable_record, "启用录像")
V(record_dev_name, "保存设备名到录像")

V(age, "年龄")

V(select_ui, "界面选择")
V(max_heart, "最大心率")

V(pulsoid_reconnect, "点击重置当前连接")
V(pulsoid_input_hint, "点击连接启动浏览器")
V(pulsoid_edit_config_hint, "也可以直接编辑文件: ")
V(pulsoid_connect, "连接")
V(pulsoid_done, "完成")
V(pulsoid_cancel, "取消")
V(pulsoid_token, "Pulsoid令牌")
V(pulsoid_clear_token, "清除并重置")

V(your_setthings_is_in_another_menu, "谢谢你玩家! 但数据源设置在另一个菜单里!")

V(ui_not_avaliable_in_qounter, "心率UI在Qounters++启用时不支持！请在Qounters++中设置HeartRate文本。")
V(ignore_qounters, "忽略Qounters++模组")
V(ignore_qounters_hint, "如果禁用了Qounters++，请启用此选项。启用后此模组可能无法与Qounters++联动。")
V(advance_ui_required, "提示:需要开启Advanced HUD才能看到这个UI")

V(mdns_enable, "启用mDNS")
V(mdns_name_no, "mDns不可用")
V(mdns_name_title, "mDns名称:")
