clc; clear; close all;

robot = TapArm_desc();
fprintf('\n========================================\n');
fprintf('即将打开 teach 窗口...\n');
fprintf('========================================\n');
q_test = [0, pi/3, 0.5, pi/3, -pi/4, pi/6];
robot.plot(q0);
robot.teach;